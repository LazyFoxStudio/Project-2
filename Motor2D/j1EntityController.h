#ifndef __j1ENTITYCONTROLLER_H__
#define __j1ENTITYCONTROLLER_H__

#include "j1Module.h"
#include "Building.h"
#include "Unit.h"
#include "Hero.h"
#include "Nature.h"

#include <list>
#include <map>

class Squad;

class j1EntityController : public j1Module
{
public:
	j1EntityController();
	~j1EntityController();

	bool Awake(pugi::xml_node&);
	bool Start();
	bool Update(float dt);
	bool PostUpdate();
	bool CleanUp();

	bool Save(pugi::xml_node&) const;
	bool Load(pugi::xml_node&);

	bool loadEntitiesDB(pugi::xml_node& data);
	
	void DeleteEntity(Entity* entity);
	bool DeleteDB() { return true; };

	void StartHero(iPoint pos);

	Unit* addUnit(iPoint pos, unitType type, Squad* squad = nullptr);
	/*Hero* addHero(iPoint pos, heroType type);*/
	Building* addBuilding(iPoint pos, buildingType type );
	Nature* addNature(iPoint pos, resourceType res_type, int amount = 0);

	Squad* AddSquad(unitType type, fPoint position);

	void selectionControl();
	void commandControl();
	entityType getSelectedType();

	bool placingBuilding(buildingType type, iPoint position);
	void buildingProcessDraw();

	void HandleWorkerAssignment(bool to_assign, Building* building);
	bool CheckCostTroop(unitType target);
	bool CheckCostBuiding(buildingType target);
	void HandleBuildingResources(buildingType target);
	void debugDrawEntity(Entity* entity);

	void GetTotalIncome();


	Entity* CheckMouseHover(iPoint mouse_world);

	std::vector<Entity*> CheckCollidingWith(SDL_Rect collider, Entity* entity_to_ignore = nullptr);
	Entity* getNearestEnemy(fPoint position, bool isEnemy);

	iPoint CameraToWorld(int x, int y);

public:

	std::list<Entity*> entities;
	std::list<Entity*> selected_entities;

	std::list<Squad*> all_squads;
	std::list<Squad*> selected_squads;

	std::list<Entity*> entities_to_destroy;

	std::map<uint, Unit*> unitDB;
	std::map<uint, Hero*> heroDB;
	std::map<uint, Building*> buildingDB;
	std::map<uint, Nature*> natureDB;

	j1Timer time_slicer;
	std::list<Entity*>::iterator entity_iterator;
	std::list<Squad*>::iterator squad_iterator;

	bool debug = false;
	bool building = false;
	int death_time = 0;
	int mill_max_villagers = 0;
	int worker_wood_production = 0;
	int test_counter = 0;
	
	buildingType structure_beingbuilt = NONE_BUILDING;
	SDL_Rect selection_rect = { 0,0,0,0 };

	Hero* hero=nullptr;
};



#endif // !

