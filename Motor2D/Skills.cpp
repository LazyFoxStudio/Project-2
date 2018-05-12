#include "Skills.h"
#include "j1Render.h"
#include "j1Input.h"
#include "j1EntityController.h"
#include "Hero.h"
#include "j1Pathfinding.h"
#include "j1Map.h"
#include "Squad.h"
#include "Effects.h"

Skill::Skill(Hero* hero, uint _radius, int _damage, uint _range, uint _cooldown, rangeType _type) : hero(hero), radius(_radius), damage(_damage), range(_range) , cooldown(_cooldown)
{
	switch (type = _type)
	{
	case AREA: tile_color = Translucid_Green;	break; 
	case LINE: tile_color = Translucid_Yellow;	break;
	case PLACE:	tile_color = Translucid_light_Blue; break;
	case HEAL:	tile_color = Translucid_Yellow; break;
	case BUFF:	tile_color = Translucid_pink; break;
	default:   tile_color = Translucid_Blue;	break; 
	}

	timer.Start();
}

void Skill::DrawRange()
{
	if(type!=PLACE)
	{
		App->render->DrawCircle(hero->position.x, hero->position.y, range, Red, true);
	}

	iPoint mouse_pos;
	App->input->GetMousePosition(mouse_pos.x, mouse_pos.y);
	cast_pos = App->render->ScreenToWorld(mouse_pos.x, mouse_pos.y);
	
	if (type == AREA || type == NONE_RANGE || type==PLACE || type== HEAL || type == BUFF)
	{
		BFS();
		
		for (std::list<iPoint>::iterator item = toDraw.begin(); item != toDraw.end(); item++)
		{
			SDL_Rect r = { (*item).x,(*item).y, App->map->data.tile_width,App->map->data.tile_height };
			App->render->DrawQuad(r, Ready() ? (cast_pos.DistanceTo(iPoint(hero->position.x, hero->position.y)) < range ? tile_color : Translucid_Grey) : Translucid_Grey);
		}

	}
	else if (type == LINE)
	{
		if (cast_pos.DistanceTo(iPoint(hero->position.x, hero->position.y)) < range)
		{
			Line();

			for (std::list<iPoint>::iterator item = toDraw.begin(); item != toDraw.end(); item++)
			{
				SDL_Rect r = { (*item).x,(*item).y, App->map->data.tile_width,App->map->data.tile_height };
				App->render->DrawQuad(r, Ready() ? tile_color : Translucid_Grey);
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
			iPoint world_tile_cast;
			iPoint world_tile = { i,j } /*= App->map->MapToWorld(i, j)*/;
			
			if (type == PLACE || type==BUFF)
			{
				world_tile_cast = App->map->WorldToMap((int)hero->position.x, (int)hero->position.y);
			}
			else 
			{
				world_tile_cast = App->map->WorldToMap(cast_pos.x, cast_pos.y);
			}

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

bool Skill::Activate()
{ 
	bool ret = false;

	if (going && App->input->GetMouseButtonDown(1) == KEY_DOWN) going = false; 

	if (!going && cast_pos.DistanceTo(iPoint(hero->position.x, hero->position.y)) < range)
	{
		hero->squad->Halt();
		iPoint cast_aux = App->map->WorldToMap(cast_pos.x, cast_pos.y);

		for (std::list<Entity*>::iterator item = App->entitycontroller->entities.begin(); item != App->entitycontroller->entities.end(); item++)
		{
			if ((*item)->IsUnit())
			{
				if ((*item)->IsEnemy())
				{
					if (type == AREA || type == NONE_RANGE || type == PLACE || type == BUFF)
					{
						if (type == PLACE || type==BUFF)
						{
							cast_aux = App->map->WorldToMap((int)hero->position.x, (int)hero->position.y);
						}

						iPoint pos = App->map->WorldToMap((*item)->position.x, (*item)->position.y);

						if (cast_aux.DistanceTo(pos) < radius)
						{
							(*item)->current_HP -= damage;

							if (type == BUFF)
							{
								((Unit*)(*item))->AddDefensebuff(15, -5, PLUS_MINUS);
							}
							
							ret = true;
						}
					}
					else if (type == LINE)
					{
						SDL_Point enemy_pos = { ((Unit*)(*item))->position.x, ((Unit*)(*item))->position.y };

						for (std::list<iPoint>::iterator it = toDraw.begin(); it != toDraw.end(); it++)
						{
							iPoint rect_point = App->map->WorldToMap((*it).x, (*it).y);
							rect_point = App->map->MapToWorld(rect_point.x, rect_point.y);
							SDL_Rect r = { rect_point.x,rect_point.y,32,32 };

							if (SDL_PointInRect(&enemy_pos, &r))
							{
								(*item)->current_HP -= damage;
								ret = true;
							}
						}
					}
				}
				else
				{
					iPoint pos = App->map->WorldToMap((*item)->position.x, (*item)->position.y);

					if (type == HEAL)
					{
						if (cast_aux.DistanceTo(pos) < radius)
						{
							(*item)->current_HP += damage;
							ret = true;
						}
					}
					else if (type == BUFF)
					{
						cast_aux = App->map->WorldToMap((int)hero->position.x, (int)hero->position.y);
						
						
						if (cast_aux.DistanceTo(pos) < radius)
						{
							LOG("defense %d", ((Unit*)(*item))->defense);
							((Unit*)(*item))->AddDefensebuff(15, 5, PLUS_MINUS);
							((Unit*)(*item))->AddPiercingDamagebuff(15, 7, PLUS_MINUS);
							ret = true;
						}
					}
				}
			}
		}
		timer.Start();
	}

	//HARDCODED Moveto()
	else if (going && last_cast.DistanceTo(iPoint(hero->position.x, hero->position.y)) < range)
	{
		going = false;
		iPoint cast_aux = App->map->WorldToMap(last_cast.x, last_cast.y);

		for (std::list<Entity*>::iterator item = App->entitycontroller->entities.begin(); item != App->entitycontroller->entities.end(); item++)
		{
			if ((*item)->IsUnit())
			{
				if ((*item)->IsEnemy())
				{
					if (type == AREA || type == NONE_RANGE)
					{
						iPoint pos = App->map->WorldToMap((*item)->position.x, (*item)->position.y);

						if (cast_aux.DistanceTo(pos) < radius)
						{
							(*item)->current_HP -= damage;
							ret = true;
						}
					}
					else if (type == LINE)
					{
						SDL_Point enemy_pos = { ((Unit*)(*item))->position.x, ((Unit*)(*item))->position.y };

						for (std::list<iPoint>::iterator it = toDraw.begin(); it != toDraw.end(); it++)
						{
							iPoint rect_point = App->map->WorldToMap((*it).x, (*it).y);
							rect_point = App->map->MapToWorld(rect_point.x, rect_point.y);
							SDL_Rect r = { rect_point.x,rect_point.y,32,32 };

							if (SDL_PointInRect(&enemy_pos, &r))
							{
								(*item)->current_HP -= damage;
								ret = true;
							}
						}
					}
				}
				else
				{
					if (type == HEAL)
					{
						iPoint pos = App->map->WorldToMap((*item)->position.x, (*item)->position.y);

						if (cast_aux.DistanceTo(pos) < radius)
						{
							(*item)->current_HP += damage;
							ret = true;
						}
					}
				}
			}
		}
		hero->squad->Halt();
		timer.Start();
	}

	else if (cast_pos.DistanceTo(iPoint(hero->position.x, hero->position.y)) > range && !going)
	{
		//go to that position if it's walkbale
		last_cast = cast_pos;
		going = true;
		iPoint map_p = App->map->WorldToMap(cast_pos.x, cast_pos.y);
		iPoint hero_map_p = App->map->WorldToMap(hero->position.x, hero->position.y);
		FlowField* shared_flowfield = App->pathfinding->RequestFlowField(map_p, hero_map_p);

		hero->squad->Halt();
		iPoint map_pos = App->map->WorldToMap(cast_pos.x, cast_pos.y);
		MoveToSquad* new_order = new MoveToSquad(hero->squad->getCommander(), map_pos);
		new_order->flow_field = shared_flowfield;
		hero->squad->commands.push_back(new_order);
		LOG("Out of range");
	}
	

	return ret;
}
