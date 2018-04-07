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
	
	bool being_built = false;
	int villagers_inside = 0;
	j1Timer timer;
	int last_frame_time = 0;

	std::vector<SDL_Rect> sprites;
	SDL_Rect current_sprite;

public:
	Building() { entity_type = BUILDING; };
	Building(iPoint pos, Building& building);
	~Building();

	void GetColliderFromSize();

	bool Update(float dt);
	void HandleSprite();
	void Draw(float dt);
};

#endif