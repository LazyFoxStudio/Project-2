#pragma once

#ifndef _BUILDING_H_
#define _BUILDING_H_

#include "Entity.h"
#include "j1Timer.h"

enum buildingType {
	NONE_BUILDING,
	TOWN_HALL,
	BARRACKS,
	LUMBER_MILL,
	FARM,
	//...
};

enum existence_state { BEING_BUILT, OPERATIVE, DESTROYED };

class Building : public Entity
{
public:
	//Stats
	buildingType type = NONE_BUILDING;
	int current_HP = 0;
	int max_HP = 0;
	int building_time = 0;
	int defense = 0;
	iPoint size = { 0,0 };
	iPoint additional_size = { 0,0 };
	uint wood_cost = 0;
	uint gold_cost = 0;
	//(...)
	
	int villagers_inside = 0;
	int resource_production = 0;

	int last_frame_time = 0;

	std::vector<SDL_Rect> sprites;
	SDL_Rect* current_sprite;
	existence_state ex_state = BEING_BUILT;

public:
	Building() { entity_type = BUILDING; };
	Building(iPoint pos, Building& building);
	~Building();

	bool Update(float dt);
	void HandleSprite();
	void HandleConstruction();
	void HandleResourceProduction();
	void CalculateResourceProduction();
	void Draw(float dt);
	void Destroy();
};

#endif