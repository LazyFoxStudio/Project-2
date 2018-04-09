#include "Skills.h"
#include "j1Render.h"
#include "j1Input.h"
#include "j1EntityController.h"
#include "Hero.h"
#include "j1Pathfinding.h"
#include "j1Map.h"

Skill::Skill(Hero* hero, uint _radius, int _damage, uint _range, uint _cooldown, rangeType _type) : hero(hero), radius(_radius), damage(_damage), range(_range) , cooldown(_cooldown)
{
	switch (type = _type)
	{
	case AREA: tile_color = { 0,255,100,100 };	break; //GREEN
	case LINE: tile_color = { 0,255,255,100 };	break; //YELLOW
	default:   tile_color = { 0,100,255,100 };	break; //BLUE
	}

	timer.Start();
}

void Skill::DrawRange()
{
	App->render->DrawCircle(hero->position.x, hero->position.y, range, Red, true);

	iPoint mouse_pos;
	App->input->GetMousePosition(mouse_pos.x, mouse_pos.y);
	cast_pos = App->render->ScreenToWorld(mouse_pos.x, mouse_pos.y);

	if (cast_pos.DistanceTo(iPoint(hero->position.x, hero->position.y)) < range)
	{
		if (type == AREA || type == NONE_RANGE)
			BFS();
		else if (type == LINE)
			Line();

		Color black = { 0,0,0,100 };
		// Draw 	
		for (std::list<iPoint>::iterator item = toDraw.begin(); item != toDraw.end(); item++)
		{
			SDL_Rect r = { (*item).x,(*item).y, App->map->data.tile_width,App->map->data.tile_height };
			if(Ready())
				App->render->DrawQuad(r, tile_color);
			else 
			{
				App->render->DrawQuad(r, black);
			}
		}
		toDraw.clear();
	}
}

void Skill::BFS()
{
	if (radius == 0) radius = 15;

	for (int i = 0; i < App->map->data.width; i++)
	{
		for (int j = 0; j < App->map->data.width; j++)
		{
			iPoint world_tile = App->map->MapToWorld(i, j);
			if (world_tile.DistanceTo(cast_pos) < radius && App->pathfinding->IsWalkable(iPoint(i, j)))
				toDraw.push_back(world_tile);
		}
	}
}

void Skill::Line()
{
	for (int i = 0; i < App->map->data.width; i++)
	{
		for (int j = 0; j < App->map->data.width; j++)
		{
			iPoint world_tile = App->map->MapToWorld(i, j);
			SDL_Rect r = { world_tile.x, world_tile.y, App->map->data.tile_width, App->map->data.tile_height };

			int hero_x = hero->position.x; int hero_y = hero->position.y;

			if (SDL_IntersectRectAndLine(&r, &hero_x, &hero_y, &cast_pos.x, &cast_pos.y) && App->pathfinding->IsWalkable(iPoint(i, j)))
				toDraw.push_back(world_tile);
		}
	}

}

void Skill::Activate()
{
	for (std::list<Entity*>::iterator item = App->entitycontroller->entities.begin(); item != App->entitycontroller->entities.end(); item++)
	{
		if ((*item)->entity_type == UNIT)
		{
			if (!((Unit*)(*item))->IsEnemy())
			{
				if((*item)->position.DistanceTo(fPoint(cast_pos.x, cast_pos.y)) < radius)
					((Unit*)(*item))->current_HP -= damage;
			}
		}
	}
	timer.Start();
}
