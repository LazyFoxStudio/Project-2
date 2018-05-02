#ifndef __j1ENTITYCONTROLLER_H__
#define __j1ENTITYCONTROLLER_H__

#include "j1Module.h"
#include "Building.h"
#include "Unit.h"
#include "Hero.h"

#include <list>
#include <map>

class Squad;
class Quadtree;

#define TOWN_HALL_POS {2000,2000}
#define MAX_VILLAGERS_LUMBERMILL 10
#define MAX_VILLAGERS_FARM 5
#define FARM_WORKER_PRODUCTION_SECONDS 5
#define DEATH_TIME 5
#define WOOD_PER_WORKER 2
#define REPAIR_COST 10
#define REPAIR_COOLDOWN 30
#define UNIT_QUEUE_MAX_SIZE 6
#define BUILDINGAREA 1750

struct worker
{
	Building* farm = nullptr;
	Building* working_at = nullptr;
	bool to_destroy = false;

	worker(Building* _farm)
	{
		farm = _farm;
		working_at = nullptr;
		to_destroy = false;
	}
};

class j1EntityController : public j1Module
{
public:
	j1EntityController();
	~j1EntityController() {};

	bool Start();
	bool Update(float dt);
	bool PostUpdate();
	bool CleanUp();

	bool Save(pugi::xml_node&) const;
	bool Load(pugi::xml_node&);

	bool loadEntitiesDB(pugi::xml_node& data);

	void DeleteEntity(uint UID);
	void DeleteSquad(uint UID);
	void DeleteDB();

	Unit* addUnit(iPoint pos, Type type, Squad* squad = nullptr);
	Hero* addHero(iPoint pos, Type type);
	Building* addBuilding(iPoint pos, Type type);
	Squad* AddSquad(Type type, fPoint position);

	void selectionControl();
	void commandControl();

	void buildingCalculations();
	bool placeBuilding(iPoint position);
	void buildingProcessDraw();

	void HandleWorkerAssignment(bool to_assign, Building* building);
	bool CheckCost(Type type);
	Cost getCost(Type type);
	bool SpendCost(Type type);
	void debugDrawEntity(Entity* entity);
	void HandleSFX(Type type, int volume = 128);
	void GetTotalIncome();
	void TownHallLevelUp();

	Entity* getEntitybyID(uint ID);
	Squad* getSquadbyID(uint ID);

	Unit* getUnitFromDB(Type type);
	Hero* getHeroFromDB(Type type);
	Building* getBuildingFromDB(Type type);

	Entity* CheckMouseHover(iPoint mouse_world);
	void CheckCollidingWith(SDL_Rect collider, std::vector<Entity*>& list_to_fill, Entity* entity_to_ignore = nullptr);

	//------Worker Related Functions--------
	void SubstractRandomWorkers(int num);
	void DeleteWorkers();
	void CreateWorkers(Building* target, int num);
	bool CheckInactiveWorkers(int num = 1);
	worker* GetInactiveWorker();
	void AssignWorker(Building* building, worker* worker);
	void DestroyWorkers();
	void UnassignRandomWorker();


public:

	std::list<Entity*> entities;
	std::list<Entity*> selected_entities;

	std::list<Squad*> squads;
	std::list<Squad*> selected_squads;

	std::vector<uint> entities_to_destroy;
	std::vector<uint> squads_to_destroy;

	std::map<uint, Entity*> DataBase;

	uint last_UID = 0;
	uint hero_UID = 0;

	Building* town_hall = nullptr;
/*
	j1Timer time_slicer;
	std::list<Entity*>::iterator entity_iterator;
	std::list<Squad*>::iterator squad_iterator;*/

	bool debug = false;

	Quadtree* colliderQT = nullptr;

	Type to_build_type = NONE_ENTITY;
	SDL_Rect selection_rect = { 0,0,0,0 };
	SDL_Rect buildingArea;

};
#endif // !

