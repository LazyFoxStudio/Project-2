#include "j1EntityController.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Scene.h"
#include "j1Gui.h"
#include "Entity.h"
#include "PugiXml/src/pugixml.hpp"
#include "j1Textures.h"
#include "j1Audio.h"
#include "j1Input.h"
#include "Hero.h"

j1EntityController::j1EntityController() { name = "entitycontroller"; }

j1EntityController::~j1EntityController() {}

bool j1EntityController::Start()
{
	pugi::xml_document doc;
	pugi::xml_node gameData;

	gameData = App->LoadFile(doc, "GameData.xml");

	loadEntitiesDB(gameData);
	addUnit(iPoint(500, 500), FOOTMAN);
	return true;
}

bool j1EntityController::Update(float dt)
{
	BROFILER_CATEGORY("Entites update", Profiler::Color::Maroon);
	if (App->map->debug) DebugDraw();

	for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)
	{
		if ((*it)->isActive)
		{
			if (App->render->CullingCam((*it)->position))	(*it)->Draw(dt);
			if (!(*it)->Update(dt))							return false;
		}
	}

	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) != KEY_IDLE)
		SelectionControl();

	return true;
}

bool j1EntityController::PostUpdate()
{
	for (std::list<Entity*>::iterator it = entities_to_destroy.begin(); it != entities_to_destroy.end(); it++)
	{
		entities.remove(*it);
		selected_entities.remove(*it);
		
		//if((*it)->collider)  DestroyCollider

		delete *it;
	}

	entities_to_destroy.clear();

	return true;
}

bool j1EntityController::CleanUp()
{
	if (!DeleteDB()) return false;

	for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)	RELEASE(*it);

	entities.clear();
	selected_entities.clear();

	return true;
}

bool j1EntityController::Save(pugi::xml_node& file) const
{
	for (std::list<Entity*>::const_iterator it = entities.begin(); it != entities.end(); it++)
		if ((*it)->Save()) return false;

	return true;
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

void j1EntityController::addUnit(iPoint pos, unitType type, Squad* squad)
{
	Unit* unit = new Unit(pos, *(unitDB[type]), squad);
	entities.push_back(unit);
	
	// if(App->render->CullingCam(unit->position))  App->audio->PlayFx(UNIT_CREATED_FX);
}

void j1EntityController::addBuilding(iPoint pos, buildingType type)
{
	//Building* building = new Building(pos, *(buildingDB[type]));
	//Switch or like Units?
}
void j1EntityController::addNature(iPoint pos, resourceType res_type, int amount)
{
	//Switch or like Units?
}

void j1EntityController::SelectionControl()
{
	int mouseX, mouseY;
	App->input->GetMousePosition(mouseX, mouseY);

	switch (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT))
	{
	case KEY_DOWN:
		selection_rect.x = mouseX; selection_rect.y = mouseY;
		break;
	case KEY_REPEAT:
		selection_rect.w = mouseX - selection_rect.x;
		selection_rect.h = mouseY - selection_rect.y;
		App->render->DrawQuad(selection_rect, White, false, false);
		break;
	case KEY_UP:

		selected_entities.clear();

		iPoint selection_to_world = App->render->ScreenToWorld(selection_rect.x, selection_rect.y);
		selection_rect.x = selection_to_world.x; selection_rect.y = selection_to_world.y;

		if (selection_rect.w == 0 || selection_rect.h == 0) selection_rect.w = selection_rect.h = 1;  // for single clicks
		else {
			if (selection_rect.w < 0) { selection_rect.w = -selection_rect.w; selection_rect.x -= selection_rect.w; }
			if (selection_rect.h < 0) { selection_rect.h = -selection_rect.h; selection_rect.y -= selection_rect.h; }
		}

		for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)
		{
			if ((*it)->entity_type == UNIT)
			{
				Unit* unit = (Unit*)*it;
				if (!unit->IsEnemy() && SDL_HasIntersection(&unit->collider, &selection_rect))
					selected_entities.push_back(unit);
			}
		}
		selection_rect = { 0,0,0,0 };
		break;
	}
}

bool j1EntityController::loadEntitiesDB(pugi::xml_node& data)
{
	pugi::xml_node NodeInfo;

	for (NodeInfo = data.child("Units").child("Unit"); NodeInfo; NodeInfo = NodeInfo.next_sibling("Unit")) {

		Unit* unitTemplate = new Unit();
		unitTemplate->type = (unitType)NodeInfo.child("type").attribute("value").as_int(0);

		if (unitTemplate->type < HERO_X)  // HERO_X should the last hero in the type enum
		{
			// TODO: hero specific functions
		}

		unitTemplate->name		= NodeInfo.child("name").attribute("value").as_string("error");
		unitTemplate->texture	= App->tex->Load(NodeInfo.child("texture").attribute("value").as_string("error"));

		unitTemplate->current_HP	= unitTemplate->max_HP = NodeInfo.child("Stats").child("life").attribute("value").as_int(0);
		unitTemplate->attack		= NodeInfo.child("Stats").child("attack").attribute("value").as_int(0);
		unitTemplate->defense		= NodeInfo.child("Stats").child("defense").attribute("value").as_int(0);
		unitTemplate->piercing_atk	= NodeInfo.child("Stats").child("piercingDamage").attribute("value").as_int(0);
		unitTemplate->speed			= NodeInfo.child("Stats").child("movementSpeed").attribute("value").as_int(0);
		unitTemplate->range			= NodeInfo.child("Stats").child("range").attribute("value").as_int(0);
		unitTemplate->line_of_sight = NodeInfo.child("Stats").child("lineOfSight").attribute("value").as_int(0);
		unitTemplate->flying		= NodeInfo.child("Stats").child("flying").attribute("value").as_bool(false);

		// TODO unit cost & cooldown, outside the DB so it's not unnecessarily repeated on every unit

		pugi::xml_node IconData;
		if (NodeInfo.child("iconData"))
			App->gui->AddIconData(unitTemplate->type, NodeInfo.child("iconData"));

		pugi::xml_node AnimInfo;
		for (AnimInfo = NodeInfo.child("Animations").child("Animation"); AnimInfo; AnimInfo = AnimInfo.next_sibling("Animation"))
		{
			Animation* animation = new Animation();
			if (animation->LoadAnimation(AnimInfo))
				unitTemplate->animations.push_back(animation);
		}

		if (!unitTemplate->animations.empty()) unitTemplate->current_anim = unitTemplate->animations.front();

		unitDB.insert(std::pair<int, Unit*>(unitTemplate->type, unitTemplate));
	}

	return true;
}