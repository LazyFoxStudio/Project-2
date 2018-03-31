#include "Skills.h"
#include "j1Render.h"
#include "Hero.h"
#include "j1Map.h"
#include "Color.h"

Shockwave::~Shockwave()
{

}

void Shockwave::Activate(Hero* hero)
{
	position = App->map->WorldToMap(hero->position.x,hero->position.y);
	//position = App->map->MapToWorld(position.x, position.y);

	DrawRange();
}

void Shockwave::DrawRange()
{

	iPoint point;

	int num_tiles = 0;

	for (int i = 0; i <= radius; i++)
	{
		num_tiles = num_tiles + 4 * i;
	}
	num_tiles = num_tiles + 1;

	BFS(num_tiles);


	// Draw 	
	for(std::list<iPoint>::iterator item = toDraw.begin();item!=toDraw.end();item++)
	{
		point = App->map->MapToWorld((*item).x,(*item).y);

		SDL_Rect r = { point.x,point.y,32,32 };
		Color tile_color = { 255,100,100,100 };

		App->render->DrawQuad(r, tile_color,point.x, point.y);
	}
}

void Shockwave::BFS(int Area)
{
	toDraw.clear();

	iPoint Goal;
	iPoint origin=position;
	
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

bool Shockwave::Find(std::list<iPoint> list,const iPoint& point)
{
	for (std::list<iPoint>::iterator it = list.begin(); it != list.end(); it++)
		if ((*it) == point) return true;

	return false;
}