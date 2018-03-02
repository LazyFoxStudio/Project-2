#include "j1EntityController.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Scene.h"
#include "j1SceneSwitch.h"
#include "Entity.h"
#include "PugiXml/src/pugixml.hpp"
#include "j1Textures.h"
#include "j1Gui.h"
#include "LabelledImage.h"

j1EntityController::j1EntityController() { name.create("entitycontroller"); }

j1EntityController::~j1EntityController() {}


bool j1EntityController::Start()
{
	//return loadEntitiesDB();
	return true;
}

bool j1EntityController::Update(float dt)
{
	BROFILER_CATEGORY("Entites update", Profiler::Color::Maroon);
	if (App->map->debug) DebugDraw();

	for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)  
		if ((*it)->Update(dt)) return false; 

	return true;
}

bool j1EntityController::PostUpdate()
{
	//for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)
	//	if (!(*it)->PostUpdate(dt)) return false;

	return true;
}

bool j1EntityController::CleanUp()
{
	for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++) RELEASE(*it);
	entities.clear();

	if (!DeleteDB()) return false;

	selected_entities.clear();

	return true;
}

bool j1EntityController::Save(pugi::xml_node& file) const
{
	for (std::list<Entity*>::const_iterator it = entities.begin(); it != entities.end(); it++)
		if ((*it)->Save()) return false;

}

bool j1EntityController::Load(pugi::xml_node& file)
{
	//TODO
	return true;
}


bool j1EntityController::DebugDraw()
{
	// TODO
	return true;
}

void j1EntityController::DeleteEntity(Entity* entity)
{
	entities.remove(entity);
	selected_entities.remove(entity);
}
