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
	}
}

void Skill::BFS()
{
	if (radius == 0) radius = 1;

	for (int i = 0; i < App->map->data.width; i++)
	{
		for (int j = 0; j < App->map->data.width; j++)
		{
			iPoint world_tile = { i,j } /*= App->map->MapToWorld(i, j)*/;
			iPoint world_tile_cast = App->map->WorldToMap(cast_pos.x, cast_pos.y);
			if (world_tile.DistanceTo(world_tile_cast) < radius && App->pathfinding->IsWalkable(iPoint(i, j)))
			{
				toDraw.push_back(App->map->MapToWorld(world_tile.x,world_tile.y));
			}
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
			int cast_pos_x = cast_pos.x;   int cast_pos_y = cast_pos.y;

			if (SDL_IntersectRectAndLine(&r, &hero_x, &hero_y, &cast_pos_x, &cast_pos_y) && App->pathfinding->IsWalkable(iPoint(i, j)))
				toDraw.push_back(world_tile);
		}
	}

}

void Skill::Activate()
{
	iPoint mouse_pos;
	App->input->GetMousePosition(mouse_pos.x, mouse_pos.y);
	cast_pos = App->render->ScreenToWorld(mouse_pos.x, mouse_pos.y);

	if (cast_pos.DistanceTo(iPoint(hero->position.x, hero->position.y)) < range)
	{
		for (std::list<Entity*>::iterator item = App->entitycontroller->entities.begin(); item != App->entitycontroller->entities.end(); item++)
		{
			if ((*item)->entity_type == UNIT)
			{
				if (((Unit*)(*item))->IsEnemy())
				{
					if (type == AREA || type == NONE_RANGE)
					{

						iPoint cast_aux = App->map->WorldToMap(cast_pos.x, cast_pos.y);
						iPoint pos = App->map->WorldToMap((*item)->position.x, (*item)->position.y);

						if (cast_aux.DistanceTo(pos) < radius)
							((Unit*)(*item))->current_HP -= damage;
					}
					else if (type==LINE)
					{
						SDL_Point enemy_pos = { ((Unit*)(*item))->position.x, ((Unit*)(*item))->position.y };

						for (std::list<iPoint>::iterator it = toDraw.begin(); it != toDraw.end(); it++)
						{
							iPoint rect_point = App->map->WorldToMap((*it).x, (*it).y);
							rect_point = App->map->MapToWorld(rect_point.x, rect_point.y);
							SDL_Rect r = { rect_point.x,rect_point.y,32,32 };

							if (SDL_PointInRect(&enemy_pos, &r))
							{
								((Unit*)(*item))->current_HP -= damage;
							}
						}
					}
				}
			}
		}
		timer.Start();
	}
}
