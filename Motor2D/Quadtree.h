#ifndef __QUADTREE_H__
#define __QUADTREE_H__ 

#define MAX_OBJECTS 5
#define MAX_LEVELS 10

#include "SDL/include/SDL_rect.h"
#include <vector>
#include <array>
#include <list>

class Entity;
struct Collider;

using namespace std;

class Quadtree
{
public:
	Quadtree()
	{
		Space = { 0,0,0,0 };

		for (int i = 0; i < Children.size(); i++)
		{
			Children[i] = nullptr;
		}

		Level = 0;
	}

	Quadtree(SDL_Rect rect, int level)
	{
		Space = rect;

		for (int i = 0; i < Children.size(); i++)
		{
			Children[i] = nullptr;
		}
		Level = level;
	}

	~Quadtree()
	{
		Clear();
	}

	void Clear();
	void Split();
	void BlitSection();
	int getIndex(const SDL_Rect& r);

	bool insert(Entity* entity);
	bool CheckBoundaries(const SDL_Rect& r)const;

	void FillCollisionVector(vector<Entity*> &EntityList, const SDL_Rect& collider);

public:

	int Level;

	SDL_Rect				Space;

	list<Entity*>			Entities;

	array<Quadtree*, 4>		Children;

};
#endif // !__QUADTREE_H__
