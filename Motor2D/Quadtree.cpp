#include "Quadtree.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Render.h"
#include "Entity.h"
//#include "j1Colliders.h"


void Quadtree::Clear()
{
	//First we have to clear all the objects of the node
	list<Entity*>::iterator item = Entities.begin();

	/*for (item; item != Entities.end(); item++)
	{
		if (*item == nullptr)
		{
			RELEASE(*item);
		}
	}*/

	Entities.clear();

	//Now we have to call all the children of the node and check if they have also childre 
	for (int i = 0; i < Children.size(); i++)
	{
		if (Children[i] != nullptr)
		{
			Children[i]->Clear();
			Children[i] = nullptr;
		}
	}

}

void Quadtree::Split()
{
	int x, y, w, h;

	x = this->Space.x;
	y = this->Space.y;
	w = (int)(this->Space.w / 2);
	h = (int)(this->Space.h / 2);

	Children[0] = new Quadtree({ x,	y, w, h }, Level + 1);
	Children[1] = new Quadtree({ x + w, y, w, h }, Level + 1);
	Children[2] = new Quadtree({ x, y + h, w, h }, Level + 1);
	Children[3] = new Quadtree({ x + w, y + h, w, h }, Level + 1);

}

// Index Quadrant
/*
---------
| 0 | 1 |
---------
| 2	| 3 |
---------
*/

int Quadtree::getIndex(const SDL_Rect& r)
{
	int index = -1;
	float MidWeight = Space.x + (Space.w / 2);
	float MidHeight = Space.y + (Space.h / 2);

	bool TopChildren = (r.y < MidHeight && r.y + r.h<MidHeight);
	bool BottomChildren = (r.y > MidHeight);

	if (r.x < MidWeight && r.x + r.w < MidWeight)
	{
		if (TopChildren)
		{
			index = 0;
		}
		else if (BottomChildren)
		{
			index = 2;
		}
	}
	else if (r.x > MidWeight)
	{
		if (TopChildren)
		{
			index = 1;
		}
		else if (BottomChildren)
		{
			index = 3;
		}
	}

	return index;
}

//TODO 3: Create the Insert() function.
bool Quadtree::insert(Entity* entity)
{
	if (entity == nullptr)
	{
		return false;
	}

	if (CheckBoundaries(entity->collider) == false)
	{
		return false;
	}

	if (Children[0] != nullptr)
	{
		int index = getIndex(entity->collider);
		if (index != -1)
		{
			Children[index]->insert(entity);
		}
		return true;
	}

	Entities.push_back(entity);

	if (Entities.size() > MAX_OBJECTS && Level<MAX_LEVELS)
	{
		if (Children[0] == nullptr)
		{
			Split();
		}

		for (list<Entity*>::iterator item = Entities.begin(); item != Entities.end(); item++)
		{
			int index = getIndex((*item)->collider);
			if (index != -1)
			{
				Children[index]->insert(*item);
				Entities.remove(*item);
			}
		}
	}

	//LOG("level: %d", Level);

	return true;
}

//TODO 3: Create the FillCollisionVector() function.
vector<Entity*> Quadtree::FillCollisionVector(vector<Entity*> &EntityList, const SDL_Rect& collider)
{
	if (Children[0] != nullptr)
	{
		int index = getIndex(collider);

		if (index != -1)
		{
			Children[index]->FillCollisionVector(EntityList, collider);
		}
		else
		{
			for (int i = 0; i < 4; ++i)
			{
				Children[i]->FillCollisionVector(EntityList, collider);
			}
		}
	}

	for (list<Entity*>::iterator item = Entities.begin(); item != Entities.end(); item++)
	{
		EntityList.push_back(*item);
	}

	return EntityList;
}

bool Quadtree::CheckBoundaries(const SDL_Rect& r)const
{
	if (Space.x < r.x + r.w && Space.x + Space.w > r.x)
	{
		if (Space.y < r.y + r.h && Space.y + Space.h > r.y)
		{
			return true;
		}
	}
	return false;
}


void Quadtree::BlitSection()
{
	if (Children[0] != nullptr)
	{
		for (int i = 0; i < 4; ++i)
		{
			Children[i]->BlitSection();
		}
	}
	else
	{
		App->render->DrawQuad(Space, Black, false);
	}
}
