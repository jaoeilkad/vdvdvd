
#include "../stdai.h"
#include "pooh_black.fdh"

#define FRAME_STAND			0
#define FRAME_DYING			1
#define FRAME_LANDED		2
#define FRAME_FLYING		3

static int bubble_xmark = 0, bubble_ymark = 0;


INITFUNC(AIRoutines)
{
	ONTICK(OBJ_POOH_BLACK, ai_pooh_black);
	ONTICK(OBJ_POOH_BLACK_BUBBLE, ai_pooh_black_bubble);

	ONTICK(OBJ_POOH_BLACK_DYING, ai_pooh_black_dying);
}

/*
void c------------------------------() {}
*/

void ai_pooh_black(Object *o)
{
	// assume flying frame as it's the one used in most of the states
	o->frame = FRAME_FLYING;
	
	//debugVline(o->CenterX(), 255,192,192);
	//debugVline(bubble_xmark, 0,0,255);
	
	switch(o->state)
	{
		case 0:
		{
			FACEPLAYER;
			o->yinertia = 0xA00;
			o->flags |= FLAG_IGNORE_SOLID;
			
			if (o->y >= (8 * TILE_H) << CSF)
			{
				o->flags &= ~FLAG_IGNORE_SOLID;
				o->state = 1;
			}
		}
		break;
		
		case 1:
		{
			o->yinertia = 0xA00;
			
			if (o->blockd)
			{
				SmokeSide(o, 8, DOWN);
				sound(SND_BIG_CRASH);
				quake(30);
				
				KillObjectsOfType(OBJ_POOH_BLACK_BUBBLE);
				o->state = 2;
			}
			
			// damage player if he falls on him
			o->damage = (o->y < player->y && player->blockd) ? 20 : 0;
		}
		break;
		
		case 2:		// landed, showing landed frame
		{
			o->frame = FRAME_LANDED;
			o->damage = 0;
			if (++o->timer > 24)
			{
				o->state = 3;
				o->timer = 0;
			}
		}
		break;
		
		case 3:		// standing, stare at player till he shoots us.
		{
			o->frame = FRAME_STAND;
			bubble_xmark = o->CenterX();
			bubble_ymark = o->CenterY();
			
			// spawn bubbles when hit
			if (o->shaketime && (o->shaketime & 1))
			{
				int x = o->CenterX() + nx_random(-12<<CSF, 12<<CSF);
				int y = o->CenterY() + nx_random(-12<<CSF, 12<<CSF);
				
				Object *bubble = CreateObject(x, y, OBJ_POOH_BLACK_BUBBLE);
				
				bubble->xinertia = nx_random(-0x600, 0x600);
				bubble->yinertia = nx_random(-0x600, 0x600);
				
				// fly away after hit enough times
				if (++o->timer > 30)
				{
					o->state = 4;
					o->timer = 0;
					
					o->flags |= FLAG_IGNORE_SOLID;
					o->yinertia = -0xC00;
				}
			}
		}
		break;
		
		case 4:		// flying away off-screen
		{
			o->timer++;
			
			// bubbles shoot down past player just before
			// he falls.
			if (o->timer == 60)
			{
				bubble_xmark = player->CenterX();
				bubble_ymark = (10000 << CSF);
			}
			else if (o->timer < 60)
			{
				bubble_xmark = o->CenterX();
				bubble_ymark = o->CenterY();
			}
			
			// fall on player
			if (o->timer >= 170)
			{
				o->x = player->CenterX() - (o->Width() / 2);
				o->y = 0;
				o->yinertia = 0x5ff;
				
				o->state = 0;
				o->timer = 0;
			}
		}
		break;
	}
	
	static const int frames[] = { 3, 3, 2, 0, 3 };
	o->frame = frames[o->state];
}


void ai_pooh_black_bubble(Object *o)
{
	if (o->hp < 100)
	{
		o->flags &= ~FLAG_SHOOTABLE;
		o->damage = 0;
		o->frame = 2;
	}
	else if (!nx_random(0, 10))
		o->frame = 0;
	else
		o->frame = 1;
	
	// adjust bubble target position so that they try to align
	// their centers with the mark instead of their upper-left corners.
	int xmark = bubble_xmark - ((sprites[SPR_POOH_BLACK_BUBBLE].w / 2) << CSF);
	int ymark = bubble_ymark - ((sprites[SPR_POOH_BLACK_BUBBLE].h / 2) << CSF);

	o->xinertia += (o->x > xmark) ? -0x40 : 0x40;
	o->yinertia += (o->y > ymark) ? -0x40 : 0x40;
	
	LIMITX(0x11FD);
	LIMITY(0x11FD);
}


void ai_pooh_black_dying(Object *o)
{
	bubble_xmark = o->CenterX();
	bubble_ymark = -(10000 << CSF);

	switch(o->state)
	{
		case 0:
		{
			o->frame = FRAME_DYING;
			FACEPLAYER;
			
			sound(SND_BIG_CRASH);
			SmokeClouds(o, 10, 12, 12);
			KillObjectsOfType(OBJ_POOH_BLACK_BUBBLE);
			
			o->state = 1;
		}
		break;
		
		case 1:
		case 2:
		{
			game.quaketime = 2;
			
			if (++o->timer > 200)
			{
				o->state = 2;
				o->timer2++;
				
				o->clip_enable = true;
				o->clipy1 = (o->timer2 / 8);
				o->display_xoff = (o->timer2 & 1);
				
				if ((o->timer2 % 4) == 2)
					sound(SND_BUBBLE);
				
				if (o->clipy1 >= o->clipy2)
					o->Delete();
			}
		}
		break;
	}
	
	
	if (o->timer & 1)
	{
		int x = o->CenterX() + nx_random(-12<<CSF, 12<<CSF);
		int y;
		
		if (o->state == 2)
			y = o->y + (o->clipy1 << CSF) + nx_random(-4<<CSF, 4<<CSF);
		else
			y = o->CenterY() + nx_random(-12<<CSF, 12<<CSF);
		
		Object *bubble   = CreateObject(x, y, OBJ_POOH_BLACK_BUBBLE);
		bubble->xinertia = nx_random(-0x200, 0x200);
		bubble->yinertia = -0x100;
	}

}




