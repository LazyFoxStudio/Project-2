#ifndef _BUILDING_H_
#define _BUILDING_H_

#include "Entity.h"
#include "j1Timer.h"

class WorkersDisplay;
class TroopCreationQueue;
class FarmWorkersManager;

enum buildingSprites   // ATTENTION!: Town hall does not use this enum yet
{
	CREATION_STARTED = 2, HALF_CREATED = 0, COMPLETE = 1, RUIN = 3
};

struct worker;

class Building : public Entity
{
public:

	iPoint size = { 0,0 };
	iPoint additional_size = { 0,0 };
	
	std::list<worker*> workers_inside;
	WorkersDisplay* workersDisplay = nullptr;
	int resource_production = 0;

	int last_frame_time = 0;

	std::vector<SDL_Rect> sprites;
	SDL_Rect* current_sprite = nullptr;
	bool producing_worker = false;
	bool producing_unit = false;
	j1Timer repair_timer;
	bool recently_repaired = false;
	std::deque<Type> unit_queue;
	TroopCreationQueue* queueDisplay = nullptr;

public:
	Building() {};
	Building(iPoint pos, Building& building);
	~Building();

	bool Update(float dt);
	void Draw(float dt);
	void turretBehavior();

	void HandleConstruction();
	void HandleResourceProduction();
	void HandleUnitProduction();
	void CalculateResourceProduction();
	void HandleWorkerProduction();

	void AddUnitToQueue(Type type);
	void RepairBuilding();
	void DemolishBuilding();
	void calculateAttackSlots(std::vector<iPoint>& list_to_fill);

	void Destroy();
};

#endif