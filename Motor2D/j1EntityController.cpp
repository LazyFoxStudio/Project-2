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

j1EntityController::j1EntityController() { name = "entitycontroller"; }

j1EntityController::~j1EntityController() {}

bool j1EntityController::Awake(pugi::xml_node &config)
{
	death_time = config.child("deathTime").attribute("value").as_int(0);
	mill_max_villagers = config.child("millMaxVillagers").attribute("value").as_int(0);
	worker_wood_production = config.child("workerWoodProduction").attribute("value").as_int(0);
	return true;
}

bool j1EntityController::Start()
{
	pugi::xml_document doc;
	pugi::xml_node gameData;

	gameData = App->LoadFile(doc, "GameData.xml");

	loadEntitiesDB(gameData);


	addUnit(iPoint(1100, 1000), GRUNT);
	addUnit(iPoint(1150, 1000), GRUNT);
	addUnit(iPoint(1200, 1000), GRUNT);
	addUnit(iPoint(1150, 1000), GRUNT);

	squad_test = AddSquad(FOOTMAN);

	//addHero(iPoint(900, 700), MAGE);

	StartHero(iPoint(900, 700));

	structure_beingbuilt = TOWN_HALL;
	placingBuilding(TOWN_HALL, {600, 600});

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
				if (debug) debugDrawEntity(*it);
			}
			if (!(*it)->Update(dt))	return false;
		}
	}

	if (App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN && App->entitycontroller->building)
	{
		building = false;
		structure_beingbuilt = NONE_BUILDING;
		App->actionscontroller->action_type = NO_ACTION;
		App->actionscontroller->doingAction = false;
	}

	if (App->input->GetKey(SDL_SCANCODE_B) == KEY_DOWN && !building && App->scene->workerAvalible())
	{
		structure_beingbuilt = BARRACKS;
		building = true;
		App->scene->inactive_workers -= 1;
	}

	else if ((App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN ) && building && structure_beingbuilt == BARRACKS && CheckCostBuiding(BARRACKS))
	{
		iPoint position;
		App->input->GetMousePosition(position.x, position.y);
	
		HandleBuildingResources(BARRACKS);
		placingBuilding(BARRACKS,position);
		if (App->actionscontroller->action_type == BUILD_BARRACKS)
			App->actionscontroller->doingAction = false;
	}

	if (App->input->GetKey(SDL_SCANCODE_M) == KEY_DOWN && !building && App->scene->workerAvalible())
	{
		structure_beingbuilt = LUMBER_MILL;
		building = true;
		App->scene->inactive_workers -= 1;
	}

	else if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN && building && structure_beingbuilt == LUMBER_MILL && CheckCostBuiding(LUMBER_MILL))
	{
		iPoint position;
		App->input->GetMousePosition(position.x, position.y);

		HandleBuildingResources(LUMBER_MILL);
		placingBuilding(LUMBER_MILL, position);
		if (App->actionscontroller->action_type == BUILD_LUMBER_MILL)
			App->actionscontroller->doingAction = false;
	}

	if (App->input->GetKey(SDL_SCANCODE_F) == KEY_DOWN && !building && App->scene->workerAvalible())
	{
		structure_beingbuilt = FARM;
		building = true;
		App->scene->inactive_workers -= 1;
	}

	else if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN && building && structure_beingbuilt == FARM && CheckCostBuiding(FARM))
	{
		iPoint position;
		App->input->GetMousePosition(position.x, position.y);

		HandleBuildingResources(FARM);
		placingBuilding(FARM, position);
		if (App->actionscontroller->action_type == BUILD_FARM)
			App->actionscontroller->doingAction = false;
	}

	if (building)
	{
		buildingProcessDraw();
	}

	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) != KEY_IDLE && !App->gui->clickedOnUI && !App->actionscontroller->doingAction)
		selectionControl();
	else if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_DOWN)
		commandControl();


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
	{
		entities.remove(*it);
		selected_entities.remove(*it);

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

Squad* j1EntityController::AddSquad(unitType type)
{
	std::vector<Unit*> squad_vector;

	for (int i = 0; i < (unitDB[type])->squad_members; ++i)
	{
		squad_vector.push_back(addUnit(iPoint((i *50) + 1000, 900), type));
	}

	Squad* new_squad = new Squad(squad_vector);
	all_squads.push_back(new_squad);
	return new_squad;
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

void j1EntityController::buildingProcessDraw()
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
	if (structure_beingbuilt == 3)
	{
		if (!App->map->WalkabilityArea((pos.x - (buildingDB[structure_beingbuilt]->additional_size.x * App->map->data.tile_width / 2)) + (buildingDB[structure_beingbuilt]->collider.w / 2), (pos.y - (buildingDB[structure_beingbuilt]->additional_size.x * App->map->data.tile_width / 2)) + (buildingDB[structure_beingbuilt]->collider.h / 2), buildingDB[structure_beingbuilt]->additional_size.x, buildingDB[structure_beingbuilt]->additional_size.y, false, true))
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
			for (std::list<Entity*>::iterator it = selected_entities.begin(); it != selected_entities.end(); it++)
				if ((*it)->entity_type == BUILDING) { selected_entities.erase(it); it--; }
		}

		App->gui->newSelectionDone();
			
		selection_rect = { 0,0,0,0 };
		break;
	}
}

Unit* j1EntityController::getNearestEnemyUnit(fPoint position, bool isEnemy)
{
	Unit* ret = nullptr;
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
		unitTemplate->speed			= NodeInfo.child("Stats").child("movementSpeed").attribute("value").as_int(0);
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

	hero->skill_one = new Skill(hero, 40, 60, 400,20, AREA);		//Icicle Crash
	hero->skill_two = new Skill(hero, 0, 200, 200,20, NONE_RANGE);	//Overflow
	hero->skill_three = new Skill(hero, 0, 50, 200, 10, LINE);		//Dragon Breath

	entities.push_back(hero);
}