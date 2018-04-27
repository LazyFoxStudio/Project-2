#ifndef __j1ENTITY_H__
#define __j1ENTITY_H__

#include "SDL/include/SDL.h"
#include "p2Point.h"
#include "p2Animation.h"
#include "p2Log.h"
#include "j1Map.h"
#include "j1App.h"

enum Type
{
	NONE_ENTITY,

	//Building
	TOWN_HALL, BARRACKS, LUMBER_MILL, FARM,

	// Heroes
	HERO_1, /* ... */ HERO_X,

	// Allies
	FOOTMAN, ARCHER, /* ... */ ALLY_X,

	//Enemies
	GRUNT, AXE_THROWER, /* ... */ ENEMY_X,
};


enum existence_state { BEING_BUILT, OPERATIVE, DESTROYED };

struct Cost
{
	uint wood_cost = 0;
	uint gold_cost = 0;
	uint oil_cost = 0;
	uint worker_cost = 0;
	uint creation_time = 0;
};

class Entity
{
public:
	fPoint position			= { 0.0f, 0.0f };
	SDL_Rect collider		= { 0,0,0,0 };
	SDL_Texture* texture	= nullptr;
	bool isActive			= true;
	Type type				= NONE_ENTITY;
	int UID					= -1;

	int current_HP			= 0;
	uint max_HP				= 0;
	uint defense			= 0;
	uint attack				= 0;
	uint piercing_atk		= 0;
	uint line_of_sight		= 0;
	uint range				= 0;

	j1Timer timer;
	Cost cost;
	std::vector<uint> available_actions;
	existence_state ex_state = BEING_BUILT;

public:
	virtual ~Entity() {};

	void setActive(bool active) { isActive = active; };

	virtual bool Update(float dt)	{ return true; };
	virtual void Draw(float dt)		{};

	virtual bool Save()		{ return true; };
	virtual bool Load()		{ return true; };

	bool IsEnemy() { return type >= GRUNT; };
	bool IsUnit() { return type >= HERO_1; };
	bool IsHero() { return (type >= HERO_1 && type <= HERO_X); }
	bool IsBuilding() { return type <= FARM; };
};

#endif