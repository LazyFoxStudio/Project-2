#ifndef __j1ENTITYCONTROLLER_H__
#define __j1ENTITYCONTROLLER_H__

#include "j1Module.h"
#include "Building.h"
#include "Unit.h"
#include "Hero.h"

#include <list>
#include <map>

class Squad;

#define TOWN_HALL_POS {2000,2000}
#define MAX_VILLAGERS_LUMBERMILL 10
#define DEATH_TIME 5
#define WOOD_PER_WORKER 2

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

	void DeleteEntity(Entity* entity);
	bool DeleteDB() { return true; };

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
	bool SpendCost(Type type);
	void debugDrawEntity(Entity* entity);
	void HandleSFX(Type type, int volume = 128);
	void GetTotalIncome();
	void TownHallLevelUp();
	void SubstractRandomWorkers(int num);

	Unit* getUnitFromDB(Type type);
	Hero* getHeroFromDB(Type type);
	Building* getBuildingFromDB(Type type);

	Entity* CheckMouseHover(iPoint mouse_world);
	std::vector<Entity*> CheckCollidingWith(SDL_Rect collider, Entity* entity_to_ignore = nullptr);
	Entity* getNearestEnemy(fPoint position, bool isEnemy);

public:

	std::list<Entity*> entities;
	std::list<Entity*> selected_entities;

	std::list<Squad*> squads;
	std::list<Squad*> selected_squads;

	std::list<Entity*> entities_to_destroy;

	std::map<uint, Entity*> DataBase;

/*
	j1Timer time_slicer;
	std::list<Entity*>::iterator entity_iterator;
	std::list<Squad*>::iterator squad_iterator;*/

	bool debug = false;

	Type to_build_type = NONE_ENTITY;
	SDL_Rect selection_rect = { 0,0,0,0 };

	Hero* hero = nullptr;
};
#endif // !

