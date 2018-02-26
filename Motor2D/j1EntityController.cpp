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

j1EntityController::j1EntityController()
{
	name.create("entitycontroller");
}

j1EntityController::~j1EntityController()
{
}

bool j1EntityController::Awake(pugi::xml_node &config)
{
	bool ret = true;
	
	return ret;
}

bool j1EntityController::Start()
{
	bool ret = true;
	//texture = App->tex->Load(PATH(folder.GetString(), texture_path.GetString()));

	return ret;
}

bool j1EntityController::Update(float dt)
{
	if (App->scene->pause) return true;

	BROFILER_CATEGORY("Entites update", Profiler::Color::Maroon);
	if (App->map->debug) DebugDraw();

	bool ret = true;
	/*p2List_item<Entity*>* tmp = Entities.start;
	while (tmp != nullptr)
	{
		ret = tmp->data->Update(dt);
		tmp = tmp->next;
	}*/

	return ret;
}

bool j1EntityController::PostUpdate()
{
	if (App->scene->pause)
		return true;

	bool ret = true;
	//p2List_item<Entity*>* tmp = Entities.start;
	//while (tmp != nullptr)
	//{
	//	tmp->data->PostUpdate();
	//	tmp = tmp->next;
	//}
	return ret;
}

bool j1EntityController::CleanUp()
{
	bool ret = true;
	//p2List_item<Entity*>* tmp = Entities.start;
	//while (tmp != nullptr)
	//{
	//	tmp->data->CleanUp();
	//	tmp = tmp->next;
	//}
	//DeleteEntities();
	return ret;
}

bool j1EntityController::Save(pugi::xml_node& file) const
{
	bool ret = true;
	/*p2List_item<Entity*>* tmp = Entities.start;
	while (tmp != nullptr)
	{
		tmp->data->Save( file);
		tmp = tmp->next;
	}*/
	return ret;
}

bool j1EntityController::Load(pugi::xml_node& file)
{
	bool ret = true;
	return ret;
}

void j1EntityController::DeleteEntities()
{
	//p2List_item<Entity*>* tmp = Entities.end;
	//while (tmp != nullptr)
	//{
	//p2List_item<Entity*>* tmp2 = tmp;
	//	RELEASE(tmp->data);
	//	Entities.del(tmp2);
	//	tmp = tmp->prev;
	}



bool j1EntityController::DebugDraw()
{
	//p2List_item<Entity*>* tmp = Entities.start;
	//while (tmp != nullptr)
	//{
	//	tmp = tmp->next;
	//}

	return true;
}

//Entity* j1EntityController::AddEntity(Entity::entityType type, iPoint position)
//{
//	Entity* tmp = nullptr;
//
//	switch (type)
//	{
//	default: break;
//	}
//
//	if (tmp) Entities.add(tmp);
//	return tmp;
//}

bool j1EntityController::DeleteEntity(Entity * entity)
{
	//entity->CleanUp();
	//Entities.del(Entities.At(Entities.find(entity)));
	return true;
}
