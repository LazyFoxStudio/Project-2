#include "Skills.h"
#include "j1Render.h"
#include "j1Input.h"
#include "j1EntityController.h"
#include "Hero.h"
#include "j1Map.h"
#include "Color.h"

void Skill::Activate(Hero* hero)
{
	position_hero = { (int)hero->position.x,(int)hero->position.y };
	position = App->map->WorldToMap(position_hero.x, position_hero.y);
	position.y += 1;
	App->input->GetMousePosition(mouse_position.x, mouse_position.y);


	if (inCircle(mouse_position.x , mouse_position.y))
	{
		mouse_position = App->map->WorldToMap(mouse_position.x - App->render->camera.x, mouse_position.y - App->render->camera.y);
		DrawRange();
		
		if (App->input->GetMouseButtonDown(1)==KEY_DOWN)
		{
			timer.Start();
			ready = false;
			MakeDamage();
		}
	}

	App->render->DrawCircle((position.x*App->map->data.tile_width) + App->render->camera.x, (position.y*App->map->data.tile_width)+App->render->camera.y, range*App->map->data.tile_width, Red);
}

void Skill::DrawRange()
{

	iPoint point;

	if (type == AREA || type == NONE_RANGE)
	{
		BFS();
	}

	if (type == LINE)
	{
		Line();
	}

	Color tile_color;

	switch (type)
	{
	case NONE_RANGE:
		tile_color = { 255,255,0,100 };//YELLOW
		break;
	case AREA:
		tile_color = { 0,255,100,100 };//GREEN
		break;
	case LINE:
		tile_color = { 100,0,255,100 };//BLUE
		break;
	default:
		tile_color = { 0,0,0,100 };//BLACK
		break;
	}
	
	// Draw 	
	for(std::list<iPoint>::iterator item = toDraw.begin();item!=toDraw.end();item++)
	{
		point = App->map->MapToWorld((*item).x,(*item).y);

		SDL_Rect r = { point.x,point.y,32,32 };

		App->render->DrawQuad(r, tile_color,point.x, point.y);
	}
}

void Skill::BFS()
{
	toDraw.clear();

	int Area = 0;

	for (int i = 0; i <= radius; i++)
	{
		Area = Area + 4 * i;
	}
	
	Area = Area + 1;

	iPoint Goal;
	iPoint origin=mouse_position;
	
	std::list<iPoint> neighbors;
	std::list<iPoint> frontier;
	
	frontier.push_back(origin);
	toDraw.push_back(origin);

	while (frontier.size() != 0 && radius>0)
	{
		origin = frontier.front();
		frontier.remove(origin);

		neighbors.push_back({ origin.x + 1, origin.y });
		neighbors.push_back({ origin.x - 1, origin.y });
		neighbors.push_back({ origin.x, origin.y + 1 });
		neighbors.push_back({ origin.x, origin.y - 1 });

		for (std::list<iPoint>::iterator item = neighbors.begin(); item!=neighbors.end();item++)
		{
			if (Find(toDraw,*item) == false)
			{
				frontier.push_back(*item);
				toDraw.push_back(*item);
			}
		}
		if (toDraw.size() >= Area)
		{
			break;
		}
	}
}

void Skill::Line()
{
	toDraw.clear();

	iPoint auxPoint = mouse_position;

	if (mouse_position.x == position.x)
	{
		toDraw.push_back(auxPoint);

		if (mouse_position.y < position.y)
		{
			while (auxPoint.y != position.y)
			{
				auxPoint.y += 1;
				toDraw.push_back(auxPoint);
			}
		}
		else
		{
			while (auxPoint.y != position.y)
			{
				auxPoint.y -= 1;
				toDraw.push_back(auxPoint);
			}
		}
	}

	else if (mouse_position.y == position.y)
	{
		toDraw.push_back(auxPoint);

		if (mouse_position.x < position.x)
		{
			while (auxPoint.x != position.x)
			{
				auxPoint.x += 1;
				toDraw.push_back(auxPoint);
			}
		}
		else
		{
			while (auxPoint.x != position.x)
			{
				auxPoint.x -= 1;
				toDraw.push_back(auxPoint);
			}
		}
	}
}

bool Skill::Find(std::list<iPoint> list,const iPoint& point)
{
	for (std::list<iPoint>::iterator it = list.begin(); it != list.end(); it++)
		if ((*it) == point) return true;

	return false;
}

void Skill::MakeDamage()
{
	for (std::list<Entity*>::iterator item = App->entitycontroller->entities.begin(); item != App->entitycontroller->entities.end(); item++)
	{
		if ((*item)->entity_type == UNIT)
		{
			if (((Unit*)(*item))->type > HERO_X)
			{
				SDL_Point enemy_pos = { ((Unit*)(*item))->position.x, ((Unit*)(*item))->position.y };

				for (std::list<iPoint>::iterator it = toDraw.begin(); it != toDraw.end(); it++)
				{
					
					iPoint rect_point = App->map->MapToWorld((*it).x, (*it).y);
					SDL_Rect r = { rect_point.x,rect_point.y,32,32 };

					if (SDL_PointInRect(&enemy_pos,&r))
					{
						((Unit*)(*item))->current_HP -= damage;
					}
				}
			}
		}
	}

}

bool Skill::inCircle(int pos_x, int pos_y)
{
	bool ret = false;

	iPoint center = { pos_x - position_hero.x-App->render->camera.x,pos_y - position_hero.y - App->render->camera.y };
	LOG("%d and %d", center.x, center.y);

	int circle_radius = (range*App->map->data.tile_width)*(range*App->map->data.tile_width);
	int circle_position = (center.x*center.x) + (center.y*center.y);

	if (circle_position<(circle_radius))
	{
		ret = true;
	}

	return ret;
}