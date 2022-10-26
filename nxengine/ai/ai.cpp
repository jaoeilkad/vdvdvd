
#include "stdai.h"
#include "ai.fdh"
#include "libretro_shared.h"

#include <streams/file_stream.h>

InitList AIRoutines;

/* forward declarations */
extern "C" {
	RFILE* rfopen(const char *path, const char *mode);
	int64_t rfseek(RFILE* stream, int64_t offset, int origin);
	int rfgetc(RFILE* stream);
	int rfclose(RFILE* stream);
	uint16_t rfgeti(RFILE *fp);
	uint32_t rfgetl(RFILE *fp);
}

bool ai_init(void)
{
	// setup function pointers to AI routines
	for(int i=0;i<OBJ_LAST;i++)
		memset(&objprop[i].ai_routines, 0, sizeof(objprop[i].ai_routines));
	
	if (load_npc_tbl()) return 1;
	
	// OBJ_NULL has flags set in npc.tbl, but shouldn't be set in our engine
	objprop[OBJ_NULL].defaultflags = 0;
	memcpy(&objprop[OBJ_SKULLHEAD_CARRIED], &objprop[OBJ_SKULLHEAD], sizeof(ObjProp));
	
	objprop[OBJ_POLISH].initial_hp = 24;	// is the value of 120 in npc.tbl really wrong? if so why?
	objprop[OBJ_POLISH].death_sound = 25;	// not sure why this is apparently wrong in file
	
	// call all the INITFUNC() routines you find at the beginning
	// of every AI-related module which assign AI logic to objects.
	if (AIRoutines.CallFunctions())
		return 1;
	
	return 0;
}


bool load_npc_tbl(void)
{
   char fname[1024];
   const int smoke_amounts[] = { 0, 3, 7, 12 };
   const int nEntries = 361;
   int i;
#ifdef _WIN32
   char slash = '\\';
#else
   char slash = '/';
#endif
   char tmp_str[256];
   snprintf(tmp_str, sizeof(tmp_str), "data%cnpc.tbl", slash);
   retro_create_path_string(fname, sizeof(fname), g_dir, tmp_str);

   RFILE *fp = rfopen(fname, "rb");
   if (!fp)
      return 1;

   for(i=0;i<nEntries;i++) objprop[i].defaultflags = rfgeti(fp);
   for(i=0;i<nEntries;i++) objprop[i].initial_hp = rfgeti(fp);

   // next is a spritesheet # of something--but we don't use it, so skip
   //for(i=0;i<nEntries;i++) fgetc(fp);		// spritesheet # or something--but we don't use it
   rfseek(fp, (nEntries * 2 * 2) + nEntries, SEEK_SET);

   for(i=0;i<nEntries;i++) objprop[i].death_sound     = rfgetc(fp);
   for(i=0;i<nEntries;i++) objprop[i].hurt_sound      = rfgetc(fp);
   for(i=0;i<nEntries;i++) objprop[i].death_smoke_amt = smoke_amounts[rfgetc(fp)];
   for(i=0;i<nEntries;i++) objprop[i].xponkill = rfgetl(fp);
   for(i=0;i<nEntries;i++) objprop[i].damage   = rfgetl(fp);

   rfclose(fp);
   return 0;//1;
}

// spawn an object at an enemies action point
Object *SpawnObjectAtActionPoint(Object *o, int otype)
{
   int x             = o->x + (sprites[o->sprite].frame[o->frame].dir[o->dir].actionpoint.x << CSF);
   int y             = o->y + (sprites[o->sprite].frame[o->frame].dir[o->dir].actionpoint.y << CSF);
   Object *newObject = CreateObject(x, y, otype);
   newObject->dir    = o->dir;
   return newObject;
}


// destroys all objects of type "otype".
// creates a BoomFlash and smoke, but no bonuses.
void KillObjectsOfType(int type)
{
	Object *o = firstobject;
	while(o)
	{
		if (o->type == type)
		{
			SmokeClouds(o, 1, 0, 0);
			effect(o->CenterX(), o->CenterY(), EFFECT_BOOMFLASH);
			
			o->Delete();
		}
		
		o = o->next;
	}
}

// deletes all objects of type "otype" silently, without any smoke or other effects.
void DeleteObjectsOfType(int type)
{
	Object *o = firstobject;
	while(o)
	{
		if (o->type == type)
			o->Delete();
		
		o = o->next;
	}
}

// handles object blinking: at random intervals forces object o's frame to blinkframe
// for blinktime frames.
void randblink(Object *o, int blinkframe, int blinktime, int prob)
{
	if (o->blinktimer)
	{
		o->blinktimer--;
		o->frame = blinkframe;
	}
	else if (nx_random(0, prob) == 0)
	{
		o->frame = blinkframe;
		o->blinktimer = 8;
	}
}

// call this in an object's aftermove routine if it's an object
// which is being carried by the player like a puppy or curly.
// x_left: offset from p's action point when he faces left
// x_right: when he faces right
// off_y: vertical offset from p's action point
void StickToPlayer(Object *o, int x_left, int x_right, int off_y)
{
   int x, y, frame;

   // needed for puppy in chest
   o->flags &= ~FLAG_SCRIPTONACTIVATE;

   // by offsetting from the player's action point, where he holds his gun, we
   // already have set up for us a nice up-and-down 1 pixel as he walks
   frame = player->frame;
   // the p's "up" frames have unusually placed action points so we have to cancel those out
   if (frame >= 3 && frame <= 5) frame -= 3;

   x = (player->x >> CSF) + sprites[player->sprite].frame[frame].dir[player->dir].actionpoint.x;
   y = (player->y >> CSF) + sprites[player->sprite].frame[frame].dir[player->dir].actionpoint.y;
   y += off_y;

   if (player->dir == RIGHT)
   {
      x += x_right;
      o->dir = RIGHT;
   }
   else
   {
      x += x_left;
      o->dir = LEFT;
   }

   o->x = (x << CSF);
   o->y = (y << CSF);
}


// used for some bosses with subobjects
void transfer_damage(Object *o, Object *target)
{
	if (o->hp < 1000)
	{
		// if you forget to set hp to 1000 when creating the puppet object,
		// it can immediately destroy the main object, possibly leading to crashes.
		target->DealDamage(1000 - o->hp);
		o->hp = 1000;
	}
}

// do the "teleport in" effect for object o.
// when complete, returns true.
// this function uses o->timer and assume o->timer starts at 0.
bool DoTeleportIn(Object *o, int slowness)
{
	if (teleffect(o, slowness, false))
	{
		o->clip_enable = false;
		return true;
	}
	
	return false;
}

// does a teleport out effect.
// When complete, returns true.
// this function uses o->timer and assume o->timer starts at 0.
bool DoTeleportOut(Object *o, int slowness)
{
	return teleffect(o, slowness, true);
}

// common code for DoTeleportIn and DoTeleportOut
// returns true when teleport is complete
static bool teleffect(Object *o, int slowness, bool teleporting_out)
{
	o->display_xoff = nx_random(-1, 1);
	
	if (!o->timer)
	{
		sound(SND_TELEPORT);
		o->clip_enable = true;
		o->clipy1 = 0;
	}
	
	if (++o->timer >= (sprites[o->sprite].h << slowness))
	{
		o->clip_enable = false;
		o->display_xoff = 0;
		return true;
	}
	else
	{
		int amt = (o->timer >> slowness);
		
		if (teleporting_out)
			o->clipy2 = sprites[o->sprite].h - amt;
		else
			o->clipy2 = amt;
		
		return false;
	}
}

/*
void c------------------------------() {}
*/

void ai_animate1(Object *o) { if (++o->frame >= sprites[o->sprite].nframes) o->frame = 0; }
void ai_animate2(Object *o) { simpleanim(o, 2); }
void ai_animate3(Object *o) { simpleanim(o, 3); }
void ai_animate4(Object *o) { simpleanim(o, 4); }
void ai_animate5(Object *o) { simpleanim(o, 5); }

static void simpleanim(Object *o, int spd)
{
	if (++o->animtimer >= spd)
	{
		o->animtimer = 0;
		if (++o->frame >= sprites[o->sprite].nframes) o->frame = 0;
	}
}

// aftermove routine which sticks the object to the action point of the NPC that's carrying it
void aftermove_StickToLinkedActionPoint(Object *o)
{
   Object *link = o->linkedobject;
   int dir;

   if (link)
   {
      dir = (link->dir ^ o->carry.flip);

      o->x = ((link->x >> CSF) + sprites[link->sprite].frame[link->frame].dir[dir].actionpoint.x) << CSF;
      o->y = ((link->y >> CSF) + sprites[link->sprite].frame[link->frame].dir[dir].actionpoint.y) << CSF;
      o->dir = dir;
   }
   else
      o->Delete();
}

void onspawn_snap_to_ground(Object *o)
{
	o->SnapToGround();
}

void onspawn_set_frame_from_id2(Object *o)
{
	o->frame = o->id2;
}
