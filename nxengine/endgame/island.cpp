
// "XX1" island-crashes cutscene from "good ending"
// and island-looks-like-it-gonna-crash but it's ok from secret ending
#include "../nx.h"
#include "island.fdh"

static struct
{
	int x, y;
	int timer, scene_length;
	int speed;
	bool survives;
	
	int scene_x, scene_y;
	int trees_x, trees_y;
} island;


bool island_init(int parameter)
{
	memset(&island, 0, sizeof(island));
	
	island.timer = 0;
	island.speed = 0x33;
	island.survives = parameter;
	island.scene_length = (!island.survives) ? 900 : 750;
	
	island.scene_x = (SCREEN_WIDTH / 2) - (sprites[SPR_ISLAND_SCENE].w / 2);
	island.scene_y = (SCREEN_HEIGHT / 2) - (sprites[SPR_ISLAND_SCENE].h / 2);
	
	island.trees_x = island.scene_x;
	island.trees_y = (island.scene_y + sprites[SPR_ISLAND_SCENE].h) - sprites[SPR_ISLAND_TREES].h;
	
	island.x = (SCREEN_WIDTH / 2) - (sprites[SPR_ISLAND].w / 2);
	island.y = (island.scene_y - sprites[SPR_ISLAND].h) << CSF;
	
	return 0;
}

/*
void c------------------------------() {}
*/

void island_tick()
{
	if (island.timer >= island.scene_length)
	{
		game.setmode(GM_NORMAL);
		return;
	}
	
	if (island.survives)
	{
		switch(island.timer)
		{
			case 350: island.speed = 0x19; break;
			case 500: island.speed = 0x0C; break;
			case 600: island.speed = 0; break;
		}
	}
	
	island.y += island.speed;
	island.timer++;
	
	// draw the scene
	Graphics::ClearScreen(BLACK);
	
	Graphics::set_clip_rect(island.scene_x, island.scene_y,
			sprites[SPR_ISLAND_SCENE].w, sprites[SPR_ISLAND_SCENE].h);
	
	Sprites::draw_sprite(island.scene_x, island.scene_y, SPR_ISLAND_SCENE);
	Sprites::draw_sprite(island.x, (island.y>>CSF), SPR_ISLAND);
	
	Sprites::draw_sprite(island.trees_x, island.trees_y, SPR_ISLAND_TREES);
	
	Graphics::clear_clip_rect();
	
	if (player->equipmask & EQUIP_NIKUMARU)
		niku_draw(game.counter);
}




/*
void c------------------------------() {}
*/




















