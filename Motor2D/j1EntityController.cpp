#include "j1EntityController.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Scene.h"
#include "j1Gui.h"
#include "Entity.h"
#include "PugiXml/src/pugixml.hpp"
#include "j1Textures.h"
#include "j1Audio.h"
#include "Command.h"
#include "j1Input.h"
#include "Squad.h"
#include "Hero.h"
#include "j1Pathfinding.h"

j1EntityController::j1EntityController() { name = "entitycontroller"; }

j1EntityController::~j1EntityController() {}

bool j1EntityController::Start()
{
	pugi::xml_document doc;
	pugi::xml_node gameData;

	gameData = App->LoadFile(doc, "GameData.xml");

	loadEntitiesDB(gameData);

	squad_units_test.push_back(addUnit(iPoint(900, 1000), FOOTMAN));
	squad_units_test.push_back(addUnit(iPoint(1000, 800), FOOTMAN));
	squad_units_test.push_back(addUnit(iPoint(1000, 1000), FOOTMAN));
	squad_units_test.push_back(addUnit(iPoint(900, 800), FOOTMAN));

	addHero(iPoint(900, 700), MAGE);

	structure_beingbuilt = TOWN_HALL;
	placingBuilding(TOWN_HALL, { 600,600 });

	squad_test = new Squad(squad_units_test);
	return true;
}

bool j1EntityController::Update(float dt)
{
	BROFILER_CATEGORY("Entites update", Profiler::Color::Maroon);

	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN) { debug = !debug; App->map->debug = debug; };

	squad_test->Update(dt);

	for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)
	{
		if ((*it)->isActive)
		{
			if (App->render->CullingCam((*it)->position))
			{
				(*it)->Draw(dt);
				if (debug) App->render->DrawQuad((*it)->collider, Green);
			}
			if (!(*it)->Update(dt))	return false;
		}
	}

	if (App->input->GetKey(SDL_SCANCODE_B) == KEY_DOWN && !building)
	{
		structure_beingbuilt = BARRACKS;
		building = true;
	}



	else if (App->input->GetKey(SDL_SCANCODE_B) == KEY_DOWN && building && structure_beingbuilt == BARRACKS)
	{
		iPoint position;
		App->input->GetMousePosition(position.x, position.y);
	
		placingBuilding(BARRACKS,position);
	}

	if (App->input->GetKey(SDL_SCANCODE_M) == KEY_DOWN && !building)
	{
		structure_beingbuilt = LUMBER_MILL;
		building = true;
	}

	else if (App->input->GetKey(SDL_SCANCODE_M) == KEY_DOWN && building && structure_beingbuilt == LUMBER_MILL)
	{
		iPoint position;
		App->input->GetMousePosition(position.x, position.y);

		placingBuilding(LUMBER_MILL, position);
	}

	if (building)
	{
		int x, y;
		App->input->GetMousePosition(x, y);
		iPoint pos = CameraToWorld(x, y);
		pos = App->map->WorldToMap(pos.x, pos.y);
		pos = App->map->MapToWorld(pos.x, pos.y);
		
		if (App->map->WalkabilityArea(pos.x, pos.y, buildingDB[structure_beingbuilt]->size.x, buildingDB[structure_beingbuilt]->size.y))
		{
			Color green = { 0,255,0,100 };
			App->render->Blit(buildingDB[structure_beingbuilt]->texture, pos.x, pos.y, &buildingDB[structure_beingbuilt]->sprites[1]);
			App->render->DrawQuad({ pos.x,pos.y,buildingDB[structure_beingbuilt]->size.x*App->map->data.tile_width,buildingDB[structure_beingbuilt]->size.y*App->map->data.tile_height }, green);
		}
		else
		{
			Color red = { 255,0,0,100 };
		
			App->render->Blit(buildingDB[structure_beingbuilt]->texture, pos.x, pos.y, &buildingDB[structure_beingbuilt]->sprites[1]);
			App->render->DrawQuad({ pos.x,pos.y,buildingDB[structure_beingbuilt]->size.x*App->map->data.tile_width,buildingDB[structure_beingbuilt]->size.y*App->map->data.tile_height }, red);
		}
		
	}


	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) != KEY_IDLE && !App->gui->clickedOnUI)
		selectionControl();
	else if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_DOWN)
		commandControl();


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


void j1EntityController::DeleteEntity(Entity* entity)
{
	entities.remove(entity);
	selected_entities.remove(entity);
}

Unit* j1EntityController::addUnit(iPoint pos, unitType type, Squad* squad)
{
	Unit* unit = new Unit(pos, *(unitDB[type]), squad);
	entities.push_back(unit);
	App->gui->createLifeBar(unit);
	
	// if(App->render->CullingCam(unit->position))  App->audio->PlayFx(UNIT_CREATED_FX);
	return unit;
}

Hero* j1EntityController::addHero(iPoint pos, heroType type)
{
	Hero* hero = new Hero(pos, *(heroDB[type]));
	entities.push_back(hero);
	App->gui->createLifeBar(hero);

	return hero;
}

Building* j1EntityController::addBuilding(iPoint pos, buildingType type)
{
	Building* building = new Building(pos, *(buildingDB[type]));
	entities.push_back(building);
	App->gui->createLifeBar(building);
	building->building_timer.Start();
	building->being_built = true;
	building->current_HP = 1;
	building->last_frame_time = 0;
	return building;
}

Nature* j1EntityController::addNature(iPoint pos, resourceType res_type, int amount)
{
	Nature* resource = new Nature(pos, *(natureDB[res_type]));
	entities.push_back(resource);
	return resource;
}

void j1EntityController::placingBuilding(buildingType type, iPoint position)
{

	iPoint pos = CameraToWorld(position.x, position.y);
	pos = App->map->WorldToMap(pos.x, pos.y);
	pos = App->map->MapToWorld(pos.x, pos.y);
	if (App->map->WalkabilityArea(pos.x, pos.y, buildingDB[structure_beingbuilt]->size.x, buildingDB[structure_beingbuilt]->size.y))
	{
		addBuilding(pos, type);
		App->map->WalkabilityArea(pos.x, pos.y, buildingDB[structure_beingbuilt]->size.x, buildingDB[structure_beingbuilt]->size.y, true);
		building = false;
	}
}

Entity* j1EntityController::CheckMouseHover(iPoint mouse_world)
{
	SDL_Point p = { mouse_world.x, mouse_world.y };

	for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)	
		if (SDL_PointInRect(&p, &(*it)->collider))  
			return (*it);

	return nullptr;
}

void j1EntityController::commandControl()
{
	int mouseX, mouseY;
	App->input->GetMousePosition(mouseX, mouseY);
	iPoint world_p = App->render->ScreenToWorld(mouseX, mouseY);
	iPoint map_p = App->map->WorldToMap(world_p.x, world_p.y);

	Entity* entity = CheckMouseHover(world_p);

	if (!entity)   // clicked on ground
	{
		squad_test->Halt();
		squad_test->commands.push_back(new MoveToSquad(squad_test->commander, map_p));
		/*for (std::list<Entity*>::iterator it = selected_entities.begin(); it != selected_entities.end(); it++)
		{
			if ((*it)->entity_type == UNIT)
			{
				Unit* unit = (Unit*)(*it);
				unit->commands.clear();
				(unit)->commands.push_back(new MoveTo(unit, map_p));
			}
		}*/
	}
	else
	{
		switch (entity->entity_type)
		{
		case UNIT:    //clicked on a unit
			if (((Unit*)entity)->IsEnemy())
			{
				for (std::list<Entity*>::iterator it = selected_entities.begin(); it != selected_entities.end(); it++)
				{
					if ((*it)->entity_type == UNIT)
					{
						Unit* unit = (Unit*)(*it);
						unit->Halt();
						unit->commands.push_back(new AttackingMoveTo(unit, map_p));
					}
				}
			}
			break;

			// TODO: other cases
		}
	}
}

void j1EntityController::selectionControl()
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
			if (SDL_HasIntersection(&(*it)->collider, &selection_rect))
			{
				if ((*it)->entity_type == UNIT)
				{
					if (!((Unit*)*it)->IsEnemy())
						selected_entities.push_back(*it);
				}
				else selected_entities.push_back(*it);
			}
		}

		if (getSelectedType() == UNIT_AND_BUILDING)
		{
			for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)
				if ((*it)->entity_type == BUILDING) entities.remove(*it);
		}

		App->gui->newSelectionDone();
			
		selection_rect = { 0,0,0,0 };
		break;
	}
}

std::vector<Entity*> j1EntityController::CheckCollidingWith(Entity* entity)
{
	std::vector<Entity*> ret;

	for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)
	{
		if (*it != entity)
			if (SDL_HasIntersection(&entity->collider, &(*it)->collider)) ret.push_back(*it);
	}

	return ret;
}

iPoint j1EntityController::CameraToWorld(int x, int y)
{
	iPoint ret;

	ret.x = x - App->render->camera.x;
	ret.y = y - App->render->camera.y;

	return ret;
}

entityType j1EntityController::getSelectedType()
{
	entityType ret = NONE_ENTITY;

	for (std::list<Entity*>::iterator it = selected_entities.begin(); it != selected_entities.end(); it++)
	{
		switch ((*it)->entity_type) 
		{
		case UNIT: 
			if (ret == NONE_ENTITY) ret = UNIT;
			else if (ret == BUILDING) ret = UNIT_AND_BUILDING;
			break;
		case BUILDING:
			if (ret == NONE_ENTITY) ret = BUILDING;
			else if (ret == UNIT) ret = UNIT_AND_BUILDING;
			break;
			//...
		default:
			break;
		}
	}

	return ret;
}

bool j1EntityController::loadEntitiesDB(pugi::xml_node& data)
{
	pugi::xml_node NodeInfo;

	for (NodeInfo = data.child("Units").child("Unit"); NodeInfo; NodeInfo = NodeInfo.next_sibling("Unit")) {

		Unit* unitTemplate = new Unit();
		unitTemplate->type = (unitType)NodeInfo.child("type").attribute("value").as_int(0);

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
		unitTemplate->wood_cost = NodeInfo.child("Stats").child("woodCost").attribute("value").as_int(0);
		unitTemplate->gold_cost = NodeInfo.child("Stats").child("goldCost").attribute("value").as_int(0);
		unitTemplate->worker_cost = NodeInfo.child("Stats").child("workerCost").attribute("value").as_int(0);
		unitTemplate->training_time = NodeInfo.child("Stats").child("trainingTime").attribute("value").as_int(0);
		unitTemplate->squad_members = NodeInfo.child("Stats").child("squadMembers").attribute("value").as_int(1);

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

	for (NodeInfo = data.child("Units").child("Hero"); NodeInfo; NodeInfo = NodeInfo.next_sibling("Hero")) {

		Hero* heroTemplate = new Hero();

		heroTemplate->type = (heroType)NodeInfo.child("type").attribute("value").as_int(0);

		if (heroTemplate->type == MAGE)  // HERO_X should the last hero in the type enum
		{
			heroTemplate->skill_one = new Skill(3, 5); //Icicle Crash
		}

		heroTemplate->name = NodeInfo.child("name").attribute("value").as_string("error");
		heroTemplate->texture = App->tex->Load(NodeInfo.child("texture").attribute("value").as_string("error"));

		heroTemplate->current_HP = heroTemplate->max_HP = NodeInfo.child("Stats").child("life").attribute("value").as_int(0);
		heroTemplate->attack = NodeInfo.child("Stats").child("attack").attribute("value").as_int(0);
		heroTemplate->defense = NodeInfo.child("Stats").child("defense").attribute("value").as_int(0);
		heroTemplate->piercing_atk = NodeInfo.child("Stats").child("piercingDamage").attribute("value").as_int(0);
		heroTemplate->speed = NodeInfo.child("Stats").child("movementSpeed").attribute("value").as_int(0);
		heroTemplate->range = NodeInfo.child("Stats").child("range").attribute("value").as_int(0);
		heroTemplate->line_of_sight = NodeInfo.child("Stats").child("lineOfSight").attribute("value").as_int(0);
		heroTemplate->wood_cost = NodeInfo.child("Stats").child("woodCost").attribute("value").as_int(0);
		heroTemplate->gold_cost = NodeInfo.child("Stats").child("goldCost").attribute("value").as_int(0);
		heroTemplate->worker_cost = NodeInfo.child("Stats").child("workerCost").attribute("value").as_int(0);
		heroTemplate->training_time = NodeInfo.child("Stats").child("trainingTime").attribute("value").as_int(0);
		heroTemplate->squad_members = NodeInfo.child("Stats").child("squadMembers").attribute("value").as_int(1);

		/*if (NodeInfo.child("iconData"))
			App->gui->AddIconData(heroTemplate->type, NodeInfo.child("iconData"));*/

		pugi::xml_node AnimInfo;
		for (AnimInfo = NodeInfo.child("Animations").child("Animation"); AnimInfo; AnimInfo = AnimInfo.next_sibling("Animation"))
		{
			Animation* animation = new Animation();
			if (animation->LoadAnimation(AnimInfo))
				heroTemplate->animations.push_back(animation);
		}

		if (!heroTemplate->animations.empty()) heroTemplate->current_anim = heroTemplate->animations.front();

		heroDB.insert(std::pair<int, Hero*>(heroTemplate->type, heroTemplate));
	}

	for (NodeInfo = data.child("Buildings").child("Building"); NodeInfo; NodeInfo = NodeInfo.next_sibling("Building")) {

		Building* buildingTemplate = new Building();
		buildingTemplate->type = (buildingType)NodeInfo.child("type").attribute("value").as_int(0);

		buildingTemplate->name = NodeInfo.child("name").attribute("value").as_string("error");
		buildingTemplate->texture = App->tex->Load(NodeInfo.child("texture").attribute("value").as_string("error"));

		buildingTemplate->current_HP = buildingTemplate->max_HP = NodeInfo.child("Stats").child("life").attribute("value").as_int(0);
		buildingTemplate->villagers_inside = NodeInfo.child("Stats").child("villagers").attribute("value").as_int(0);
		buildingTemplate->building_time = NodeInfo.child("Stats").child("buildingTime").attribute("value").as_int(0);
		buildingTemplate->defense = NodeInfo.child("Stats").child("defense").attribute("value").as_int(0);
		buildingTemplate->size.x = NodeInfo.child("size").attribute("x").as_int(0);
		buildingTemplate->size.y = NodeInfo.child("size").attribute("y").as_int(0);
		// TODO building cost outside the DB so it's not unnecessarily repeated on every unit

		pugi::xml_node IconData;
		if (NodeInfo.child("iconData"))
			App->gui->AddIconData(buildingTemplate->type, NodeInfo.child("iconData"));

		pugi::xml_node SpriteInfo;
		for (SpriteInfo = NodeInfo.child("sprites").child("sprite"); SpriteInfo; SpriteInfo = SpriteInfo.next_sibling("sprite"))
		{
			SDL_Rect tmp;
			tmp.x = SpriteInfo.attribute("x").as_int(0);
			tmp.y = SpriteInfo.attribute("y").as_int(0);
			tmp.w = SpriteInfo.attribute("w").as_int(0);
			tmp.h = SpriteInfo.attribute("h").as_int(0);

			buildingTemplate->sprites.push_back(tmp);
		}

		buildingDB.insert(std::pair<int, Building*>(buildingTemplate->type, buildingTemplate));
	}

	for (NodeInfo = data.child("Nature").child("Resource"); NodeInfo; NodeInfo = NodeInfo.next_sibling("Resource")) {

		Nature* natureTemplate = new Nature();
		natureTemplate->type = (resourceType)NodeInfo.child("type").attribute("value").as_int(0);

		natureTemplate->name = NodeInfo.child("name").attribute("value").as_string("error");
		natureTemplate->texture = App->tex->Load(NodeInfo.child("texture").attribute("value").as_string("error"));

		// TODO IconData for mines
		pugi::xml_node IconData;
		if (NodeInfo.child("iconData") && natureTemplate->type == GOLD)
			App->gui->AddIconData(natureTemplate->type, NodeInfo.child("iconData"));

		natureDB.insert(std::pair<int, Nature*>(natureTemplate->type, natureTemplate));
	}

	return true;
}