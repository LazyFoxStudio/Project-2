#ifndef __j1ENTITYCONTROLLER_H__
#define __j1ENTITYCONTROLLER_H__

#include "j1Module.h"

#include <vector>
#include <map>

#include "Building.h"
#include "Unit.h"
#include "Nature.h"

class j1EntityController : public j1Module
{
public:
	j1EntityController();
	~j1EntityController();

	bool Awake(pugi::xml_node & config);
	bool Start();
	bool Update(float dt);
	bool PostUpdate();
	bool CleanUp();

	bool Save(pugi::xml_node&) const;
	bool Load(pugi::xml_node&);
	bool DebugDraw();
	
	bool DeleteEntity(Entity* entity);
	void DeleteEntities();

	void addUnit(unitType type, iPoint pos, Squad* squad = nullptr);
	void addBuilding(buildingType type, iPoint pos);
	void addNature(resourceType res_type, int amount);

	void placingBuilding(buildingType type);

public:

	std::vector<Entity*> entities;
	std::vector<Entity*> selected_entities;
	
	std::map<int, Unit*> unitDB;
	std::map<int, Building*> buildingDB;
	std::map<int, Nature*> natureDB;

	bool godmode = false;

};



#endif // !

