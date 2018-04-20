#pragma once

#ifndef _BUILDING_H_
#define _BUILDING_H_

#include "Entity.h"
#include "j1Timer.h"

enum buildingSprites   // ATTENTION!: Town hall does not use this enum yet
{
	CREATION_STARTED = 2, HALF_CREATED = 0, COMPLETE = 1, RUIN = 3
};

class Building : public Entity
{
public:

	iPoint size = { 0,0 };
	iPoint additional_size = { 0,0 };
	
	int villagers_inside = 0;
	int resource_production = 0;

	int last_frame_time = 0;

	std::vector<SDL_Rect> sprites;
	SDL_Rect* current_sprite = nullptr;

public:
	Building() {};
	Building(iPoint pos, Building& building);
	~Building() { sprites.clear(); };

	bool Update(float dt);
	void Draw(float dt);

	void HandleConstruction();
	void HandleResourceProduction();
	void CalculateResourceProduction();

	void Destroy();
};

#endif