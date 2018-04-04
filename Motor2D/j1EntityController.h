#ifndef __j1ENTITYCONTROLLER_H__
#define __j1ENTITYCONTROLLER_H__

#include "j1Module.h"
#include "Building.h"
#include "Unit.h"
#include "Nature.h"

#include <list>
#include <map>

class Squad;

class j1EntityController : public j1Module
{
public:
	j1EntityController();
	~j1EntityController();

	bool Start();
	bool Update(float dt);
	bool PostUpdate();
	bool CleanUp();

	bool Save(pugi::xml_node&) const;
	bool Load(pugi::xml_node&);

	bool loadEntitiesDB(pugi::xml_node& data);
	
	void DeleteEntity(Entity* entity);
	bool DeleteDB() { return true; };

	Unit* addUnit(iPoint pos, unitType type, Squad* squad = nullptr);
	Building* addBuilding(iPoint pos, buildingType type );
	Nature* addNature(iPoint pos, resourceType res_type, int amount = 0);


	void selectionControl();
	void commandControl();
	entityType getSelectedType();

	void placingBuilding(buildingType type, iPoint position);
	Entity* CheckMouseHover(iPoint mouse_world);
	std::vector<Entity*> CheckCollidingWith(Entity* entity);

	iPoint CameraToWorld(int x, int y);

public:

	std::list<Entity*> entities;
	std::list<Entity*> selected_entities;

	std::list<Entity*> entities_to_destroy;
	
	std::vector<Unit*> squad_units_test;  // for testing purposes, will be deleted
	Squad* squad_test = nullptr; // for testing purposes, will be deleted

	std::map<uint, Unit*> unitDB;
	std::map<uint, Building*> buildingDB;
	std::map<uint, Nature*> natureDB;

	bool debug = false;
	bool building = false;
	SDL_Rect selection_rect = { 0,0,0,0 };
};



#endif // !

