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
#include "j1Map.h"
#include "j1Pathfinding.h"
#include "j1Map.h"
#include "j1ActionsController.h"
#include "UI_WarningMessages.h"
#include "UI_Button.h"

#define SQUAD_MAX_FRAMETIME 0.1f
#define ENITITY_MAX_FRAMETIME 0.3f


j1EntityController::j1EntityController() { name = "entitycontroller"; }

j1EntityController::~j1EntityController() {}

bool j1EntityController::Awake(pugi::xml_node &config)
{
	death_time = config.child("deathTime").attribute("value").as_int(0);
	mill_max_villagers = config.child("millMaxVillagers").attribute("value").as_int(0);
	worker_wood_production = config.child("workerWoodProduction").attribute("value").as_int(0);

	entity_iterator = entities.begin();
	squad_iterator = all_squads.begin();
	return true;
}

bool j1EntityController::Start()
{
	pugi::xml_document doc;
	pugi::xml_node gameData;

	gameData = App->LoadFile(doc, "GameData.xml");

	loadEntitiesDB(gameData);

	AddSquad(FOOTMAN, fPoint(1900, 2100));
	AddSquad(FOOTMAN, fPoint(2200, 2100));

	AddSquad(GRUNT, fPoint(3000, 2100));
	AddSquad(AXE_THROWER, fPoint(1700, 2100));
	AddSquad(ARCHER, fPoint(1500, 2100));


	StartHero(iPoint(2000, 1950));

	structure_beingbuilt = TOWN_HALL;
	placingBuilding(TOWN_HALL, {2000, 2000});

	return true;
}

bool j1EntityController::Update(float dt)
{
	BROFILER_CATEGORY("Entites update", Profiler::Color::Maroon);

	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN) { debug = !debug; App->map->debug = debug; };

	
	int counter = 0;
	time_slicer.Start();
	while (time_slicer.Read() < ((float)App->framerate * SQUAD_MAX_FRAMETIME) && counter < all_squads.size())
	{
		counter++; squad_iterator++;
		if(squad_iterator == all_squads.end()) squad_iterator = all_squads.begin();
		if (!(*squad_iterator)->Update(dt))							return false;
	}

	counter = 0;
	time_slicer.Start();
	while (time_slicer.Read() < ((float)App->framerate * ENITITY_MAX_FRAMETIME) && counter < entities.size())
	{
		counter++; entity_iterator++;
		if (entity_iterator == entities.end()) entity_iterator = entities.begin();
		if ((*entity_iterator) != nullptr)
		{
			if ((*entity_iterator)->isActive)
				if (!(*entity_iterator)->Update(dt))	DeleteEntity(*entity_iterator);
		}
	}

	for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)
	{
		if ((*it)->isActive)
		{
			if (App->render->CullingCam((*it)->position))
			{
				(*it)->Draw(dt);
				if (debug) debugDrawEntity(*it);
			}
		}
	}

	if ((App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN ) && building && App->scene->workerAvalible() && App->entitycontroller->CheckCostBuiding(structure_beingbuilt))
	{
		iPoint position;
		App->input->GetMousePosition(position.x, position.y);
		if (placingBuilding(structure_beingbuilt, position))
		{
			//Hardcoded
			Button* barracks = App->gui->GetActionButton(5);
			barracks->Unlock();
			Button* farms = App->gui->GetActionButton(7);
			farms->Unlock();

			App->gui->warningMessages->hideMessage(NO_TREES);
			HandleBuildingResources(structure_beingbuilt);

			App->scene->inactive_workers -= 1;
			if (App->actionscontroller->action_type == structure_beingbuilt && App->input->GetKey(SDL_SCANCODE_LSHIFT) != KEY_DOWN && App->input->GetKey(SDL_SCANCODE_LSHIFT) != KEY_REPEAT)
				App->actionscontroller->doingAction = false;
		}
	}

	if (building)
	{
		buildingProcessDraw();
	}

	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) != KEY_IDLE && !App->gui->clickedOnUI && !App->actionscontroller->doingAction_lastFrame)
		selectionControl();
	else if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_DOWN && !App->actionscontroller->doingAction_lastFrame)
		commandControl();

	if ((App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN || App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT)) && App->entitycontroller->building)
	{
		building = false;
		structure_beingbuilt = NONE_BUILDING;
		App->actionscontroller->action_type = NO_ACTION;
		App->actionscontroller->doingAction = false;
		App->gui->warningMessages->hideMessage(NO_WORKERS);
		App->gui->warningMessages->hideMessage(NO_RESOURCES);
		App->gui->warningMessages->hideMessage(NO_TREES);
	}

	return true;
}

void j1EntityController::debugDrawEntity(Entity* entity)
{
	App->render->DrawQuad(entity->collider, Green);
	if (entity->entity_type == UNIT)
	{
		Unit* unit = (Unit*)entity;
		App->render->DrawCircle(unit->position.x, unit->position.y, unit->range, Green);
		App->render->DrawCircle(unit->position.x, unit->position.y, unit->line_of_sight, Blue);
	}
}

bool j1EntityController::PostUpdate()
{
	for (std::list<Entity*>::iterator it = entities_to_destroy.begin(); it != entities_to_destroy.end(); it++)
		DeleteEntity(*it);

	entities_to_destroy.clear();

	return true;
}

bool j1EntityController::CleanUp()
{
	if (!DeleteDB()) return false;

	for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)	
		DeleteEntity(*it);

	entities.clear();
	selected_entities.clear();

	all_squads.clear();
	selected_squads.clear();

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
	App->gui->entityDeleted(entity);

	Unit * unit_to_remove = nullptr;
	switch (entity->entity_type)
	{
	case UNIT:
		unit_to_remove = (Unit*)(entity);
		unit_to_remove->squad->removeUnit(unit_to_remove);
		if ((unit_to_remove)->squad->units.empty())
		{
			all_squads.remove(unit_to_remove->squad);
			selected_squads.remove(unit_to_remove->squad);
			RELEASE(unit_to_remove->squad);
		}
		delete unit_to_remove;
		break;
	case BUILDING: delete ((Building*)(entity)); break;
	case NATURE: delete ((Nature*)(entity)); break;
	}
}

Unit* j1EntityController::addUnit(iPoint pos, unitType type, Squad* squad)
{
	Unit* unit = new Unit(pos, *(unitDB[type]), squad);
	if (!unit->IsEnemy())
	{
		App->scene->workers--;
		App->scene->inactive_workers--;
	}
	entities.push_back(unit);
	App->gui->createLifeBar(unit);
	
	// if(App->render->CullingCam(unit->position))  App->audio->PlayFx(UNIT_CREATED_FX);
	return unit;
}

//Hero* j1EntityController::addHero(iPoint pos, heroType type)
//{
//	Hero* hero = new Hero(pos, *(heroDB[type]));
//	entities.push_back(hero);
//	App->gui->createLifeBar(hero);
//
//	return hero;
//}

Building* j1EntityController::addBuilding(iPoint pos, buildingType type)
{
	Building* building = new Building(pos, *(buildingDB[type]));
	entities.push_back(building);
	App->gui->createLifeBar(building);
	building->timer.Start();
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

Squad* j1EntityController::AddSquad(unitType type, fPoint position)
{
	std::vector<Unit*> squad_vector;
	std::vector<iPoint> positions;
	iPoint map_p = App->map->WorldToMap(position.x, position.y);
	Squad* new_squad = nullptr;

	if (App->pathfinding->GatherWalkableAdjacents(map_p, (unitDB[type])->squad_members, positions))
	{
		for (int i = 0; i < (unitDB[type])->squad_members; ++i)
		{
			iPoint world_p = App->map->MapToWorld(positions[i].x, positions[i].y);
			squad_vector.push_back(addUnit(world_p, type));
		}
		new_squad = new Squad(squad_vector);
		all_squads.push_back(new_squad);
	}
	return new_squad;
}

bool j1EntityController::placingBuilding(buildingType type, iPoint position)
{
	iPoint pos;
	bool ret = false;
	if (building)
	{
		pos = App->render->ScreenToWorld(position.x, position.y);
	}
	else
	{
		pos = position;// = App->render->ScreenToWorld(position.x, position.y);
	}

	pos = App->map->WorldToMap(pos.x, pos.y);
	pos = App->map->MapToWorld(pos.x, pos.y);
	SDL_Rect building_col = { pos.x, pos.y, buildingDB[structure_beingbuilt]->size.x*App->map->data.tile_width, buildingDB[structure_beingbuilt]->size.y*App->map->data.tile_height };

	if (App->map->WalkabilityArea(pos.x, pos.y, buildingDB[structure_beingbuilt]->size.x, buildingDB[structure_beingbuilt]->size.y) && App->entitycontroller->CheckCollidingWith(building_col).empty())
	{
		addBuilding(pos, type);
		App->map->WalkabilityArea(pos.x, pos.y, buildingDB[structure_beingbuilt]->size.x, buildingDB[structure_beingbuilt]->size.y, true,false);
		building = false;
		ret = true;
	}

	return ret;
}

void j1EntityController::buildingProcessDraw()
{

	int x, y;
	App->input->GetMousePosition(x, y);
	iPoint pos = CameraToWorld(x, y);
	pos = App->map->WorldToMap(pos.x, pos.y);
	pos = App->map->MapToWorld(pos.x, pos.y);

	if (!App->scene->workerAvalible())
		App->gui->warningMessages->showMessage(NO_WORKERS);
	else
		App->gui->warningMessages->hideMessage(NO_WORKERS);

	if (!CheckCostBuiding(structure_beingbuilt))
		App->gui->warningMessages->showMessage(NO_RESOURCES);
	else
		App->gui->warningMessages->hideMessage(NO_RESOURCES);

	if (App->map->WalkabilityArea(pos.x, pos.y, buildingDB[structure_beingbuilt]->size.x, buildingDB[structure_beingbuilt]->size.y) && CheckCostBuiding(structure_beingbuilt) && App->scene->workerAvalible())
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
	if (structure_beingbuilt == 3)
	{
		bool treesAround = !App->map->WalkabilityArea((pos.x - (buildingDB[structure_beingbuilt]->additional_size.x * App->map->data.tile_width / 2)) + (buildingDB[structure_beingbuilt]->collider.w / 2), (pos.y - (buildingDB[structure_beingbuilt]->additional_size.x * App->map->data.tile_width / 2)) + (buildingDB[structure_beingbuilt]->collider.h / 2), buildingDB[structure_beingbuilt]->additional_size.x, buildingDB[structure_beingbuilt]->additional_size.y, false, true);
		
		if (!treesAround)
			App->gui->warningMessages->showMessage(NO_TREES);
		else
			App->gui->warningMessages->hideMessage(NO_TREES);
		
		if (treesAround && CheckCostBuiding(structure_beingbuilt) && App->scene->workerAvalible())
		{
			Color green2 = { 0,255,0,75 };
			App->render->DrawQuad({ (pos.x - (buildingDB[structure_beingbuilt]->additional_size.x * App->map->data.tile_width / 2)) + (buildingDB[structure_beingbuilt]->collider.w / 2),(pos.y - (buildingDB[structure_beingbuilt]->additional_size.x * App->map->data.tile_width / 2)) + (buildingDB[structure_beingbuilt]->collider.h / 2),buildingDB[structure_beingbuilt]->additional_size.x*App->map->data.tile_width,buildingDB[structure_beingbuilt]->additional_size.y*App->map->data.tile_height }, green2);
		}
		else
		{
			Color red2 = { 255,0,0,75 };
			App->render->DrawQuad({ (pos.x - (buildingDB[structure_beingbuilt]->additional_size.x * App->map->data.tile_width / 2)) + (buildingDB[structure_beingbuilt]->collider.w / 2),(pos.y - (buildingDB[structure_beingbuilt]->additional_size.x * App->map->data.tile_width / 2)) + (buildingDB[structure_beingbuilt]->collider.h / 2),buildingDB[structure_beingbuilt]->additional_size.x*App->map->data.tile_width,buildingDB[structure_beingbuilt]->additional_size.y*App->map->data.tile_height }, red2);
		}
	}

}

void j1EntityController::HandleWorkerAssignment(bool to_assign, Building * building)
{
	if (building->type == LUMBER_MILL)
	{
		if (to_assign)
		{
			if (building->villagers_inside < mill_max_villagers && App->scene->workerAvalible())
			{
				building->villagers_inside += 1;
				App->scene->inactive_workers -= 1;
			}
			else
			{
				//send sfx
			}
		}
		else
		{
			if (building->villagers_inside > 0)
			{
				building->villagers_inside -= 1;
				App->scene->inactive_workers += 1;
			}
		}
	}
}

bool j1EntityController::CheckCostTroop(unitType target)
{
	return App->scene->wood >= unitDB[target]->wood_cost && App->scene->gold >= unitDB[target]->gold_cost && App->scene->workerAvalible(unitDB[target]->squad_members);
}

bool j1EntityController::CheckCostBuiding(buildingType target)
{
	bool ret = false;

		if (App->scene->wood >= buildingDB[target]->wood_cost && App->scene->gold >= buildingDB[target]->gold_cost)
		{
			ret = true;
		}

	return ret;
}

void j1EntityController::HandleBuildingResources(buildingType target)
{
	App->scene->wood -= buildingDB[target]->wood_cost;
	App->scene->gold -= buildingDB[target]->gold_cost;
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
		for (std::list<Squad*>::iterator it = selected_squads.begin(); it != selected_squads.end(); it++)
		{
			(*it)->Halt();
			(*it)->commands.push_back(new MoveToSquad((*it)->commander, map_p));
		}
	}
	else
	{
		switch (entity->entity_type)
		{
		case UNIT:    //clicked on a unit
			if (((Unit*)entity)->IsEnemy())
			{
				for (std::list<Squad*>::iterator it = selected_squads.begin(); it != selected_squads.end(); it++)
				{
					(*it)->Halt();
					(*it)->commands.push_back(new AttackingMoveToSquad((*it)->commander, map_p));
				}
			}
			break;

			// TODO: other cases
		}
	}
}

bool CompareSquad(Squad* s1, Squad* s2)
{
	return s1 == s2;
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
		selected_squads.clear();

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
					{	
						if(((Unit*)*it)->squad)
							selected_squads.push_back(((Unit*)*it)->squad);
					}
				}
				else selected_entities.push_back(*it);
				if ((*it)->entity_type == BUILDING)
				{
					App->actionscontroller->newSquadPos = { (*it)->position.x, (*it)->position.y + (*it)->collider.h };
				}
			}
		}

		selected_squads.unique(CompareSquad);

		for (std::list<Squad*>::iterator it = selected_squads.begin(); it != selected_squads.end(); it++)
			for(int i = 0; i < (*it)->units.size(); i++)
				selected_entities.push_back((*it)->units[i]);
		
		if (getSelectedType() == UNIT_AND_BUILDING)
		{
			for (std::list<Entity*>::iterator it = selected_entities.begin(); it != selected_entities.end(); it++)
				if ((*it)->entity_type == BUILDING) { selected_entities.erase(it); it--; }
		}

		App->gui->newSelectionDone();
			
		selection_rect = { 0,0,0,0 };
		break;
	}
}

Entity* j1EntityController::getNearestEnemy(fPoint position, bool isEnemy)
{
	Entity* ret = nullptr;
	for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)
	{
		if ((*it)->entity_type == UNIT)
		{
			Unit* enemy = (Unit*)(*it);
			if (enemy->IsEnemy() != isEnemy)
			{
				if (!ret) { ret = enemy; continue; }
				else if (enemy->position.DistanceTo(position) < ret->position.DistanceTo(position)) ret = enemy;
			}
		}
		else if ((*it)->entity_type == BUILDING && isEnemy)
		{
			if (!ret) { ret = (*it); continue; }
			else if ((*it)->position.DistanceTo(position) < ret->position.DistanceTo(position)) ret = (*it);
		}
	}
	return ret;
}

std::vector<Entity*> j1EntityController::CheckCollidingWith(SDL_Rect collider, Entity* entity_to_ignore)
{
	std::vector<Entity*> ret;

	for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)
	{
		if (*it != entity_to_ignore)
			if (SDL_HasIntersection(&collider, &(*it)->collider)) ret.push_back(*it);
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
		unitTemplate->speed			= NodeInfo.child("Stats").child("movementSpeed").attribute("value").as_float(0.0f);
		unitTemplate->range			= NodeInfo.child("Stats").child("range").attribute("value").as_int(0);
		unitTemplate->line_of_sight = NodeInfo.child("Stats").child("lineOfSight").attribute("value").as_int(0);
		unitTemplate->flying		= NodeInfo.child("Stats").child("flying").attribute("value").as_bool(false);
		unitTemplate->wood_cost = NodeInfo.child("Stats").child("woodCost").attribute("value").as_int(0);
		unitTemplate->gold_cost = NodeInfo.child("Stats").child("goldCost").attribute("value").as_int(0);
		unitTemplate->worker_cost = NodeInfo.child("Stats").child("workerCost").attribute("value").as_int(0);
		unitTemplate->training_time = NodeInfo.child("Stats").child("trainingTime").attribute("value").as_int(0);
		unitTemplate->squad_members = NodeInfo.child("Stats").child("squadMembers").attribute("value").as_int(1);
		int i = 0;
		for (pugi::xml_node action = NodeInfo.child("Actions").child("action"); action; action = action.next_sibling("action"))
		{
			if (i >= 9)
				break;

			unitTemplate->available_actions[i++] = action.attribute("id").as_uint();
		}

		int size_x = NodeInfo.child("Stats").child("size").attribute("x").as_int(1);
		int size_y = NodeInfo.child("Stats").child("size").attribute("y").as_int(1);

		unitTemplate->collider = { 0,0, App->map->data.tile_width * size_x, App->map->data.tile_height * size_y };

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

	for (NodeInfo = data.child("Buildings").child("Building"); NodeInfo; NodeInfo = NodeInfo.next_sibling("Building")) {

		Building* buildingTemplate = new Building();
		buildingTemplate->type = (buildingType)NodeInfo.child("type").attribute("value").as_int(0);

		buildingTemplate->name = NodeInfo.child("name").attribute("value").as_string("error");
		buildingTemplate->texture = App->tex->Load(NodeInfo.child("texture").attribute("value").as_string("error"));

		buildingTemplate->current_HP = buildingTemplate->max_HP = NodeInfo.child("Stats").child("life").attribute("value").as_int(0);
		buildingTemplate->villagers_inside = NodeInfo.child("Stats").child("villagers").attribute("value").as_int(0);
		buildingTemplate->building_time = NodeInfo.child("Stats").child("buildingTime").attribute("value").as_int(0);
		buildingTemplate->defense = NodeInfo.child("Stats").child("defense").attribute("value").as_int(0);
		buildingTemplate->wood_cost = NodeInfo.child("Stats").child("woodCost").attribute("value").as_int(0);
		buildingTemplate->gold_cost = NodeInfo.child("Stats").child("goldCost").attribute("value").as_int(0);
		buildingTemplate->size.x = NodeInfo.child("size").attribute("x").as_int(0);
		buildingTemplate->size.y = NodeInfo.child("size").attribute("y").as_int(0);
		buildingTemplate->additional_size.x = NodeInfo.child("additionalSize").attribute("x").as_int(0);
		buildingTemplate->additional_size.y = NodeInfo.child("additionalSize").attribute("y").as_int(0);
		buildingTemplate->GetColliderFromSize();
		// TODO building cost outside the DB so it's not unnecessarily repeated on every unit
		int i = 0;
		for (pugi::xml_node action = NodeInfo.child("Actions").child("action"); action; action = action.next_sibling("action"))
		{
			if (i >= 9)
				break;

			buildingTemplate->available_actions[i++] = action.attribute("id").as_uint();
		}

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

void j1EntityController::StartHero(iPoint pos)
{
	hero = new Hero();

	hero->name = (unitDB[HERO_1])->name;
	hero->texture = (unitDB[HERO_1])->texture;
	hero->collider = (unitDB[HERO_1])->collider;
	hero->type = (unitDB[HERO_1])->type;
	
	hero->attack = (unitDB[HERO_1])->attack;
	hero->current_HP = hero->max_HP = (unitDB[HERO_1])->max_HP;
	hero->defense = (unitDB[HERO_1])->defense;
	hero->piercing_atk = (unitDB[HERO_1])->piercing_atk;
	hero->speed = (unitDB[HERO_1])->speed;
	hero->line_of_sight = (unitDB[HERO_1])->line_of_sight;
	hero->range = (unitDB[HERO_1])->range;

	for (int i = 0; i < (unitDB[HERO_1])->animations.size(); i++)
		hero->animations.push_back(new Animation(*(unitDB[HERO_1])->animations[i]));

	hero->current_anim = hero->animations[0];

	hero->entity_type = UNIT;

	hero->position.x = hero->collider.x = pos.x;
	hero->position.y = hero->collider.y = pos.y;

	hero->skill_one = new Skill(hero, 40, 60, 400, 20, AREA);		//Icicle Crash
	hero->skill_two = new Skill(hero, 0, 200, 200, 20, NONE_RANGE);	//Overflow
	hero->skill_three = new Skill(hero, 0, 50, 200, 10, LINE);		//Dragon Breath

	std::vector<Unit*>aux_vector;
	aux_vector.push_back(hero);

	Squad* new_squad = new Squad(aux_vector);
	all_squads.push_back(new_squad);

	App->gui->createLifeBar(hero);

	entities.push_back(hero);
}