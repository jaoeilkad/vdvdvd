
#include "../stdai.h"
#include "final_misc.fdh"

INITFUNC(AIRoutines)
{
	ONTICK(OBJ_MIMIGA_CAGED, ai_mimiga_caged);
	ONTICK(OBJ_CHIE_CAGED, ai_mimiga_caged);
	ONTICK(OBJ_CHACO_CAGED, ai_mimiga_caged);
	ONTICK(OBJ_SANTA_CAGED, ai_mimiga_caged);
	
	ONTICK(OBJ_DOCTOR_GHOST, ai_doctor_ghost);
	ONTICK(OBJ_RED_ENERGY, ai_red_energy);
}

/*
void c------------------------------() {}
*/

// The Doctor in his red energy form.
// there is no "move" state, when he takes over Misery,
// the object is moved kind of unconventionally, using an <MNP.
void ai_doctor_ghost(Object *o)
{
	switch(o->state)
	{
		case 10:
		{
			o->state = 11;
			o->timer = 0;
		}
		case 11:
		{
			Object *r = CreateObject(o->x, o->y+(128<<CSF), OBJ_RED_ENERGY);
			r->angle = RIGHT;
			r->linkedobject = o;
			
			if (++o->timer > 150)
				o->state++;
		}
		break;
		
		case 20:
		{
			o->state = 21;
			o->timer = 0;
		}
		case 21:
		{
			if (++o->timer > 250)
				DeleteObjectsOfType(OBJ_RED_ENERGY);
		}
		break;
	}
}

// red energy for doctor. In a completely different role,
// it's also used for the dripping blood from Ballos's final form.
void ai_red_energy(Object *o)
{

	switch(o->angle)
	{
		case UP:
		{
			o->yinertia -= 0x40;
			if (o->blocku && o->yinertia < 0) o->Delete();
		}
		break;
		
		case DOWN:
		{
			o->yinertia += 0x40;
			if (o->blockd && o->yinertia > 0) o->Delete();
			
			if (++o->timer > 50)
				o->Delete();
			
			if (o->yinertia > 0x5ff)
				o->yinertia = 0x5ff;
		}
		break;
		
		case RIGHT:
		{
			if (!o->linkedobject) { o->Delete(); return; }
			
			if (o->state == 0)
			{
				o->state = 1;
				o->flags |= FLAG_IGNORE_SOLID;
				
				o->xinertia = nx_random(-0x600, 0x600);
				o->yinertia = nx_random(-0x600, 0x600);
				
				// accel speed
				o->speed = (512 / nx_random(16, 51));
				
				// x/y limit
				int limit = nx_random(0x80, 0x100);
				
				o->timer2 = (limit * 2);	// x limit
				o->timer3 = (limit * 3);	// y limit (form elongated sphere)
			}
			
			int tgtx = o->linkedobject->x + (4<<CSF);
			if (o->x < tgtx) 		o->xinertia += o->speed;
			else if (o->x > tgtx)	o->xinertia -= o->speed;
			
			if (o->y < o->linkedobject->y) 		o->yinertia += o->speed;
			else if (o->y > o->linkedobject->y) o->yinertia -= o->speed;
			
			LIMITX(o->timer2);
			LIMITY(o->timer3);
		}
	}
	
	o->frame = nx_random(0, 1);
}


/*
void c------------------------------() {}
*/


void ai_mimiga_caged(Object *o)
{
	switch(o->state)
	{
		case 0:
		{
			o->state = 1;
			o->x -= (1 << CSF);
			o->y -= (2 << CSF);
			
			switch(o->type)
			{
				case OBJ_CHIE_CAGED: o->sprite = SPR_CHIE; break;
				case OBJ_CHACO_CAGED: o->sprite = SPR_CHACO; break;
				
				case OBJ_SANTA_CAGED:
					o->sprite = SPR_SANTA;
					o->x += (2 << CSF);
				break;
				
				default:
					o->sprite = SPR_MIMIGA_CAGED;
					o->x += (2 << CSF);
				break;
			}
		}
		case 1:
		{
			o->frame = 0;
			randblink(o, 1, 12, 160);
			
			if (o->frame == 0)
				FACEPLAYER;
		}
		break;
		
		
		case 10:	// blush and spawn heart
		{
			o->state = 11;
			o->frame = 2;
			
			Object *h = CreateObject(o->x, o->y-(16<<CSF), OBJ_HEART);
			h->state = 1;	// not map-spawned (disappear after a moment)
		}
		case 11:
		{
			FACEPLAYER;
		}
		break;
	}
}








