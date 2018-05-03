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
#include "j1WaveController.h"
#include "UI_Button.h"
#include "Building.h"
#include "Quadtree.h"
#include "UI_InfoTable.h"

#define SQUAD_MAX_FRAMETIME 0.1f
#define ENITITY_MAX_FRAMETIME 0.3f

#define MOUSE_RADIUS 15 //(in pixels)

j1EntityController::j1EntityController() { name = "entitycontroller"; }

bool j1EntityController::Start()
{
	colliderQT = new Quadtree({ 0,0,App->map->data.width*App->map->data.tile_width,App->map->data.height*App->map->data.tile_height }, 0);

	addHero(iPoint(2000, 1950), HERO_1);

	iPoint town_hall_pos = TOWN_HALL_POS;
	town_hall = addBuilding(town_hall_pos, TOWN_HALL);
	App->map->WalkabilityArea(town_hall_pos.x, town_hall_pos.y, town_hall->size.x, town_hall->size.y, true, false);
	App->scene->InitialWorkers(town_hall);

	//AddSquad(FOOTMAN, fPoint(2200, 1950));

	buildingArea.w = BUILDINGAREA;
	buildingArea.h = BUILDINGAREA;
	//buildingArea.x = -BUILDINGAREA / 2 + town_hall_pos.x / 2;
	//buildingArea.y = -BUILDINGAREA / 2 + town_hall_pos.y / 2;
	buildingArea.x = town_hall_pos.x - (BUILDINGAREA/2) + (town_hall->size.x*App->map->data.tile_width/2);
	buildingArea.y = town_hall_pos.y - (BUILDINGAREA / 2) + (town_hall->size.x*App->map->data.tile_height/2);
/*
	entity_iterator = entities.begin();
	squad_iterator = all_squads.begin();*/
	return true;
}

bool j1EntityController::Update(float dt)
{
	BROFILER_CATEGORY("Entites update", Profiler::Color::Maroon);

	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN) { debug = !debug; App->map->debug = debug; };
	Hero* hero = (Hero*)getEntitybyID(hero_UID);

	for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)
	{
		if ((*it)->isActive || (*it) == hero)
		{
			colliderQT->insert(*it);

			if (!App->scene->toRestart)
			{
				if (!(*it)->Update(dt))	entities_to_destroy.push_back((*it)->UID);
			}
			
			if (App->render->CullingCam((*it)->position))
			{
				(*it)->Draw(dt);
				if (debug) debugDrawEntity(*it);
			}
		}
	}

	for (std::list<Squad*>::iterator it = squads.begin(); it != squads.end() && !App->scene->toRestart; it++)
	{
		if (!(*it)->Update(dt)) squads_to_destroy.push_back((*it)->UID);
	}

	if (App->scene->toRestart) return true;

	/*int counter = 0;
	if (!all_squads.empty())
	{
		time_slicer.Start();
		while (time_slicer.Read() < ((float)App->framerate * SQUAD_MAX_FRAMETIME) && counter < all_squads.size())
		{
			counter++; squad_iterator++;
			if (squad_iterator == all_squads.end()) squad_iterator = all_squads.begin();
			if (!(*squad_iterator)->Update(dt))							return false;
		}
	}

	if (!entities.empty())
	{
		counter = 0;
		time_slicer.Start();
		while (time_slicer.Read() < ((float)App->framerate * ENITITY_MAX_FRAMETIME) && counter < entities.size())
		{
			counter++; entity_iterator++;
			if (entity_iterator == entities.end()) entity_iterator = entities.begin();

			if ((*entity_iterator)->isActive || (*entity_iterator) == hero)
				if (!(*entity_iterator)->Update(dt))	entities_to_destroy.push_back(*entity_iterator);

		}
	}*/

	if (to_build_type != NONE_ENTITY)
		buildingCalculations();

	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) != KEY_IDLE && !App->actionscontroller->doingAction_lastFrame && (hero ? hero->current_skill == 0 : true) && !App->gui->leftClickedOnUI)
		selectionControl();
	else if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_DOWN && !App->actionscontroller->doingAction_lastFrame && !App->gui->rightClickedOnUI)
		commandControl();

	if ((App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN || App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT)) && to_build_type != NONE_ENTITY && App->actionscontroller->doingAction)
	{
		to_build_type = NONE_ENTITY;
		App->actionscontroller->activateAction(NO_ACTION);
	}

	if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && town_hall != nullptr)
	{
		for (std::list<Entity*>::iterator it_e = selected_entities.begin(); it_e != selected_entities.end(); it_e++)
			(*it_e)->isSelected = false;
		selected_entities.clear();

		selected_entities.push_back(town_hall);
		town_hall->isSelected = true;
		App->gui->newSelectionDone();
	}

	return true;
}

void j1EntityController::buildingCalculations()
{
	buildingProcessDraw();

	if ((App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN) && !App->gui->leftClickedOnUI)
	{
		if (CheckInactiveWorkers() && App->entitycontroller->CheckCost(to_build_type))
		{
			iPoint position;
			App->input->GetMousePosition(position.x, position.y);
			if (placeBuilding(position))
			{
				//Hardcoded
				Button* barracks = App->gui->GetActionButton(5);
				barracks->Unlock();
				Button* farms = App->gui->GetActionButton(7);
				farms->Unlock();

				App->gui->warningMessages->hideMessage(NO_TREES);
				App->entitycontroller->SpendCost(to_build_type);

				if (App->actionscontroller->action_type == to_build_type && App->input->GetKey(SDL_SCANCODE_LSHIFT) != KEY_DOWN && App->input->GetKey(SDL_SCANCODE_LSHIFT) != KEY_REPEAT)
					App->actionscontroller->doingAction = false;

				to_build_type = NONE_ENTITY;
			}
		}
	}
}

void j1EntityController::debugDrawEntity(Entity* entity)
{
	App->render->DrawQuad(entity->collider, Green);
	if (entity->IsUnit())
	{
		Unit* unit = (Unit*)entity;
		SDL_Rect r = { unit->position.x - unit->range, unit->position.y - unit->range, unit->range * 2, unit->range * 2};

		App->render->DrawQuad(r, White, false);
		App->render->DrawCircle(unit->position.x, unit->position.y, unit->line_of_sight, Blue);
		App->render->DrawLine(unit->position.x, unit->position.y, unit->position.x + unit->next_step.x, unit->position.y + unit->next_step.y, Red);
		if (unit->squad)
		{
			fPoint offset = unit->squad->getOffset(unit->UID);
			App->render->DrawCircle(offset.x + unit->squad->centroid.x, offset.y + unit->squad->centroid.y, 5, Yellow);
		}
	}
}

void j1EntityController::HandleSFX(Type type, int volume)
{
	switch (type)
	{
	case NONE_ENTITY:
		break;
	case HERO_1:
		App->audio->PlayFx(SFX_HERO_YHAMAM_BASICATTACK, volume);
		break;
	case FOOTMAN:
		App->audio->PlayFx(SFX_MISCELLANEOUS_SWORD_CLASH, volume);
		break;
	case ARCHER:
		App->audio->PlayFx(SFX_MISCELLANEOUS_ARROW, volume);
		break;
	case KNIGHT:
		App->audio->PlayFx(SFX_MISCELLANEOUS_ARROW, volume);
	case GRUNT:
		// May be changed if a better sfx is found (for free)
		App->audio->PlayFx(SFX_MISCELLANEOUS_SWORD_CLASH, volume);
		break;
	case AXE_THROWER:
		App->audio->PlayFx(SFX_MISCELLANEOUS_AXETHROW, volume);
		break;
	default:
		break;
	}
}
void j1EntityController::GetTotalIncome()
{
	App->scene->wood_production_per_second = 0;
	for (std::list<Entity*>::iterator tmp = entities.begin(); tmp != entities.end(); tmp++)
	{
		if ((*tmp)->IsBuilding())
		{
			if ((*tmp)->type == LUMBER_MILL && (*tmp)->ex_state != DESTROYED)
				App->scene->wood_production_per_second += ((Building*)(*tmp))->resource_production;			
		}		
	}
}

bool j1EntityController::PostUpdate()
{
	BROFILER_CATEGORY("Entites postupdate", Profiler::Color::Maroon);

	int selected_size = selected_entities.size();

	for (int i = 0; i < entities_to_destroy.size(); i++)
	{
		if (Entity* entity = getEntitybyID(entities_to_destroy[i]))
		{
			DeleteEntity(entities_to_destroy[i]);
			entities.remove(getEntitybyID(entities_to_destroy[i]));		
		}
	}

	for (int i = 0; i < squads_to_destroy.size(); i++)
	{
		DeleteSquad(squads_to_destroy[i]);
		squads.remove(getSquadbyID(squads_to_destroy[i]));
	}

	if(selected_size != selected_entities.size())
		App->gui->newSelectionDone();

	entities_to_destroy.clear();
	squads_to_destroy.clear();

	DestroyWorkers();

	if(debug)
		colliderQT->BlitSection();

	colliderQT->Clear();
	return true;
}

bool j1EntityController::CleanUp()
{
	for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)
	{
		App->gui->entityDeleted(*it);
		DeleteEntity((*it)->UID);
	}

	for (std::list<Squad*>::iterator it = squads.begin(); it != squads.end(); it++)
		DeleteSquad((*it)->UID);

	DeleteDB();

	entities_to_destroy.clear();
	squads_to_destroy.clear();

	selected_entities.clear();
	selected_squads.clear();

	entities.clear();
	squads.clear();

	last_UID = 0;
	RELEASE(colliderQT);
/*
	entity_iterator = entities.begin();
	squad_iterator = squads.begin();*/

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


void j1EntityController::DeleteEntity(uint UID)
{
	if (Entity* entity = getEntitybyID(UID))
	{/*
		if(entity == *entity_iterator)
			entity_iterator = entities.begin();*/

		selected_entities.remove(entity);

		if (entity->IsUnit())
		{
			Unit* unit_to_remove = (Unit*)(entity);

			if (unit_to_remove->IsHero())
			{
				Hero* hero_to_remove = (Hero*)unit_to_remove;
				RELEASE(hero_to_remove);
			}
			else
				{ RELEASE(unit_to_remove); }
		}
		else   // is building
		{
			Building* building_to_remove = (Building*)(entity);
			App->map->WalkabilityArea(building_to_remove->position.x, building_to_remove->position.y, building_to_remove->size.x, building_to_remove->size.y, true);

			if(!App->scene->toRestart)
				App->wavecontroller->updateFlowField();

			RELEASE(building_to_remove);
		}
	}
}

void j1EntityController::DeleteSquad(uint UID)
{
	if (Squad* squad = getSquadbyID(UID))
	{
		selected_squads.remove(squad);

		squad->Destroy();
		RELEASE(squad);
	}
}

Entity* j1EntityController::getEntitybyID(uint ID)
{
	for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)
		if ((*it)->UID == ID) return *it;
	
	return nullptr;
}

Squad* j1EntityController::getSquadbyID(uint ID)
{
	for (std::list<Squad*>::iterator it = squads.begin(); it != squads.end(); it++)
		if ((*it)->UID == ID) return *it;

	return nullptr;
}

Unit* j1EntityController::addUnit(iPoint pos, Type type, Squad* squad)
{
	Unit* unit = new Unit(pos, *getUnitFromDB(type), squad);

	unit->UID = last_UID++;
	entities.push_back(unit);
	App->gui->createLifeBar(unit);
	
	// if(App->render->CullingCam(unit->position))  App->audio->PlayFx(UNIT_CREATED_FX);
	return unit;
}


Hero* j1EntityController::addHero(iPoint pos, Type type)
{
	Hero* hero = new Hero();
	hero_UID = hero->UID = last_UID++;
	Hero* hero_template = getHeroFromDB(type);

	hero->texture	= hero_template->texture;
	hero->collider	= hero_template->collider;
	hero->type	= hero_template->type;

	hero->attack		= hero_template->attack;
	hero->current_HP	= hero->max_HP = hero_template->max_HP;
	hero->defense		= hero_template->defense;
	hero->piercing_atk	= hero_template->piercing_atk;
	hero->speed			= hero_template->speed;
	hero->line_of_sight = hero_template->line_of_sight;
	hero->range			= hero_template->range;

	for (int i = 0; i < hero_template->animations.size(); i++)
		hero->animations.push_back(new Animation(*hero_template->animations[i]));

	hero->current_anim = hero->animations[0];

	for (int i = 0; i < 9; i++)
		hero->available_actions = hero_template->available_actions;

	hero->infoData = hero_template->infoData;

	hero->position.x = pos.x;
	hero->position.y = pos.y;

	hero->collider.x = pos.x - (hero->collider.w / 2);
	hero->collider.y = pos.y - (hero->collider.h / 2);

	if (type == HERO_1)
	{
		hero->skill_one = new Skill(hero, 3, 100, 300, 6, AREA);		//Icicle Crash
		hero->skill_two = new Skill(hero, 0, 400, 700, 2, NONE_RANGE);	//Overflow
		hero->skill_three = new Skill(hero, 0, 200, 250, 2, LINE);		//Dragon Breath
	}

	App->gui->createLifeBar(hero);

	entities.push_back(hero);

	std::vector<uint>aux_vector;
	aux_vector.push_back(hero->UID);

	Squad* new_squad = new Squad(aux_vector);
	new_squad->UID = last_UID++;
	squads.push_back(new_squad);

	return hero;
}

Building* j1EntityController::addBuilding(iPoint pos, Type type)
{
	Building* building = new Building(pos, *getBuildingFromDB(type));
	building->UID = last_UID++;
	if (type == LUMBER_MILL)
		building->workersDisplay = App->gui->createWorkersDisplay(building);
	else if (type == BARRACKS || type == GNOME_HUT || type == CHURCH)
		building->queueDisplay = App->gui->createTroopCreationQueue(building);
	/*else if (type == FARM)
		building->workersManager = App->gui->createWorkersManager(building);*/
	entities.push_back(building);
	App->gui->createLifeBar(building);

	if (type != TOWN_HALL)
		building->current_HP = 1;

	return building;
}

Squad* j1EntityController::AddSquad(Type type, fPoint position)
{
	std::vector<uint> squad_vector;
	std::vector<iPoint> positions;
	iPoint map_p = App->map->WorldToMap(position.x, position.y);
	Squad* new_squad = nullptr;
	Unit* unit_template = getUnitFromDB(type);

	if (App->pathfinding->GatherWalkableAdjacents(map_p, getUnitFromDB(type)->squad_members, positions))
	{
		for (int i = 0; i < unit_template->squad_members; ++i)
		{
			iPoint world_p = App->map->MapToWorld(positions[i].x, positions[i].y);
			squad_vector.push_back(addUnit(world_p, type)->UID);
		}
		new_squad = new Squad(squad_vector);
		if (!unit_template->IsEnemy())
		{
			App->scene->wood -= unit_template->cost.wood_cost;
			SubstractRandomWorkers(unit_template->cost.worker_cost);
		}
		new_squad->UID = last_UID++;
		squads.push_back(new_squad);
	}
	return new_squad;
}

Unit* j1EntityController::getUnitFromDB(Type type)
{
	return (DataBase[type]->IsUnit() ? (Unit*)DataBase[type] : nullptr);
}

Hero* j1EntityController::getHeroFromDB(Type type)
{
	return (DataBase[type]->IsHero() ? (Hero*)DataBase[type] : nullptr);
}

Building* j1EntityController::getBuildingFromDB(Type type)
{
	return (DataBase[type]->IsBuilding() ? (Building*)DataBase[type] : nullptr);
}


bool j1EntityController::placeBuilding(iPoint position)
{
	iPoint pos = App->render->ScreenToWorld(position.x, position.y);
	pos = App->map->WorldToMap(pos.x, pos.y);
	pos = App->map->MapToWorld(pos.x, pos.y);

	Building* to_build = getBuildingFromDB(to_build_type);
	SDL_Rect building_col = { pos.x, pos.y, to_build->size.x*App->map->data.tile_width, to_build->size.y*App->map->data.tile_height };

	std::vector<Entity*> collisions;
	App->entitycontroller->CheckCollidingWith(building_col, collisions);

	if (App->map->WalkabilityArea(pos.x, pos.y, to_build->size.x, to_build->size.y) && collisions.empty() && SDL_HasIntersection(&building_col,&buildingArea))
	{
		Building* tmp = addBuilding(pos, to_build_type);
		worker* tmp2 = GetInactiveWorker();
		
		tmp->workers_inside.push_back(tmp2);
		tmp2->working_at = tmp;
		
		App->map->WalkabilityArea(pos.x, pos.y, to_build->size.x, to_build->size.y, true,false);
		App->wavecontroller->updateFlowField();

		return true;
	}

	return false;
}

void j1EntityController::buildingProcessDraw()
{
	App->render->DrawQuad(buildingArea, Transparent_Blue);
	iPoint pos = { 0,0 };
	App->input->GetMousePosition(pos.x, pos.y);
	pos = App->render->ScreenToWorld(pos.x, pos.y);
	pos = App->map->WorldToMap(pos.x, pos.y);
	pos = App->map->MapToWorld(pos.x, pos.y);
	Building* to_build = getBuildingFromDB(to_build_type);
	SDL_Rect building_col = { pos.x, pos.y, to_build->size.x*App->map->data.tile_width, to_build->size.y*App->map->data.tile_height };
	bool enough_resources = true;

	if (!CheckInactiveWorkers()) { App->gui->warningMessages->showMessage(NO_WORKERS); enough_resources = false; }
	else								App->gui->warningMessages->hideMessage(NO_WORKERS);

	if (!CheckCost(to_build_type)) { App->gui->warningMessages->showMessage(NO_RESOURCES); enough_resources = false; }
	else							App->gui->warningMessages->hideMessage(NO_RESOURCES);

	App->gui->warningMessages->hideMessage(NO_TREES);


	if (App->map->WalkabilityArea(pos.x, pos.y, to_build->size.x, to_build->size.y) && enough_resources&& SDL_HasIntersection(&building_col, &buildingArea))
		App->render->DrawQuad({ pos.x,pos.y,to_build->size.x*App->map->data.tile_width,to_build->size.y*App->map->data.tile_height }, Translucid_Green);
	else
		App->render->DrawQuad({ pos.x,pos.y,to_build->size.x*App->map->data.tile_width,to_build->size.y*App->map->data.tile_height }, Red);

	if (to_build_type == LUMBER_MILL)
	{
		bool treesAround = !App->map->WalkabilityArea((pos.x - (to_build->additional_size.x * App->map->data.tile_width / 2)) + (to_build->collider.w / 2), (pos.y - (to_build->additional_size.x * App->map->data.tile_width / 2)) + (to_build->collider.h / 2), to_build->additional_size.x, to_build->additional_size.y, false, true);

		if (!treesAround)
			App->gui->warningMessages->showMessage(NO_TREES);

		if (treesAround && enough_resources)
			App->render->DrawQuad({ (pos.x - (to_build->additional_size.x * App->map->data.tile_width / 2)) + (to_build->collider.w / 2),(pos.y - (to_build->additional_size.x * App->map->data.tile_width / 2)) + (to_build->collider.h / 2),to_build->additional_size.x*App->map->data.tile_width, to_build->additional_size.y*App->map->data.tile_height }, Transparent_Green);
		else
			App->render->DrawQuad({ (pos.x - (to_build->additional_size.x * App->map->data.tile_width / 2)) + (to_build->collider.w / 2),(pos.y - (to_build->additional_size.x * App->map->data.tile_width / 2)) + (to_build->collider.h / 2),to_build->additional_size.x*App->map->data.tile_width,to_build->additional_size.y*App->map->data.tile_height }, Transparent_Red);
	}



	App->render->Blit(to_build->texture, pos.x, pos.y, &to_build->sprites[COMPLETE]);

}

void j1EntityController::HandleWorkerAssignment(bool to_assign, Building * building)
{	
	if (building)
	{
		if (building->type == LUMBER_MILL)
		{
			if (to_assign)
			{
				if (building->workers_inside.size() < MAX_VILLAGERS_LUMBERMILL && CheckInactiveWorkers())
				{
					worker* tmp = GetInactiveWorker();
					
					AssignWorker(building, tmp);
				}
				else
				{
					//send sfx
				}
			}
			else
			{
				if (building->workers_inside.size()>0)
				{
					building->workers_inside.back()->working_at = nullptr;
					building->workers_inside.pop_back();
				}
			}
			building->CalculateResourceProduction();
			GetTotalIncome();
		}
	}
}

bool j1EntityController::CheckCost(Type target)
{
	return (App->scene->wood >= DataBase[target]->cost.wood_cost && App->scene->gold >= DataBase[target]->cost.gold_cost && ((!DataBase[target]->IsUnit()) || CheckInactiveWorkers(DataBase[target]->cost.worker_cost)));
}

Cost j1EntityController::getCost(Type target)
{
	if (DataBase.size() >= target)
		return DataBase[target]->cost;
	else
		return Cost();
}

bool j1EntityController::SpendCost(Type target)
{
	if (CheckCost(target)) {
		App->scene->wood -= DataBase[target]->cost.wood_cost;
		App->scene->gold -= DataBase[target]->cost.gold_cost;
		return true;
	}
	return false;
}

Entity* j1EntityController::CheckMouseHover(iPoint mouse_world)
{
	SDL_Rect r = { mouse_world.x - (MOUSE_RADIUS / 2), mouse_world.y - (MOUSE_RADIUS / 2) , MOUSE_RADIUS , MOUSE_RADIUS };
	Entity* ret = nullptr;

	std::vector<Entity*> hovered_entities;
	for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)	
		if (SDL_HasIntersection(&r, &(*it)->collider))
			hovered_entities.push_back(*it);

	if (!hovered_entities.empty())
	{
		ret = hovered_entities.front();
		for (int i = 1; i < hovered_entities.size(); i++)
		{
			if (hovered_entities[i]->position.DistanceTo(fPoint(mouse_world.x, mouse_world.y)) < ret->position.DistanceTo(fPoint(mouse_world.x, mouse_world.y)))
				ret = hovered_entities[i];
		}
	}

	return ret;
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
		if (!selected_squads.empty())
		{
			FlowField* shared_flowfield = App->pathfinding->RequestFlowField(map_p);
			for (std::list<Squad*>::iterator it = selected_squads.begin(); it != selected_squads.end(); it++)
			{
				(*it)->Halt();
				MoveToSquad* new_order = new MoveToSquad((*it)->getCommander(), map_p);
				new_order->flow_field = shared_flowfield;
				(*it)->commands.push_back(new_order);
			}
			shared_flowfield->used_by = selected_squads.size();
		}
	}
	else
	{
		if (entity->IsEnemy() && entity->ex_state != DESTROYED && entity->isActive)    //clicked on a enemy
		{
			FlowField* shared_flowfield = App->pathfinding->RequestFlowField(map_p);
			for (std::list<Squad*>::iterator it = selected_squads.begin(); it != selected_squads.end(); it++)
			{
				(*it)->Halt();
				AttackingMoveToSquad* new_order = nullptr;
				new_order = new AttackingMoveToSquad((*it)->getCommander(), map_p, false, ((Unit*)entity)->squad->UID);

				new_order->flow_field = shared_flowfield;
				(*it)->commands.push_back(new_order);
			}
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

		for (std::list<Entity*>::iterator it_e = selected_entities.begin(); it_e != selected_entities.end(); it_e++)
			(*it_e)->isSelected = false;
		selected_entities.clear();
		selected_squads.clear();
		bool buildings = false;
		bool units = false;

		iPoint selection_to_world = App->render->ScreenToWorld(selection_rect.x, selection_rect.y);
		selection_rect.x = selection_to_world.x; selection_rect.y = selection_to_world.y;

		if (selection_rect.w == 0 && selection_rect.h == 0) selection_rect.w = selection_rect.h = 1;  // for single clicks
		else {
			if (selection_rect.w < 0) { selection_rect.w = -selection_rect.w; selection_rect.x -= selection_rect.w; }
			if (selection_rect.h < 0) { selection_rect.h = -selection_rect.h; selection_rect.y -= selection_rect.h; }
		}

		for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)
		{
			if ((*it)->isActive) {

				if (SDL_HasIntersection(&(*it)->collider, &selection_rect))
				{
					if ((*it)->IsUnit())
					{
						if (!(*it)->IsEnemy())
						{
							selected_squads.push_back(((Unit*)*it)->squad);
							if (!units) units = true;
						}
					}
					else // is building
					{
						if ((*it)->ex_state == OPERATIVE)
						{
							selected_entities.push_back(*it);
							(*it)->isSelected = true;
							App->actionscontroller->newSquadPos = { (*it)->position.x, (*it)->position.y + (*it)->collider.h };
							if (!buildings) buildings = true;
						}
					}
				}
			}
		}

		selected_squads.unique(CompareSquad);

		for (std::list<Squad*>::iterator it = selected_squads.begin(); it != selected_squads.end(); it++)
		{
			std::vector<Unit*> units;
			(*it)->getUnits(units);

			for (int i = 0; i < units.size(); i++)
			{
				selected_entities.push_back(units[i]);
				units[i]->isSelected = true;
			}
		}

		if (buildings && units)
		{
			for (std::list<Entity*>::iterator it = selected_entities.begin(); it != selected_entities.end(); it++)
				if ((*it)->IsBuilding()) { selected_entities.erase(it); it--; }
		}

		App->gui->newSelectionDone();
			
		selection_rect = { 0,0,0,0 };
		break;
	}
}



void j1EntityController::CheckCollidingWith(SDL_Rect collider, std::vector<Entity*>& list_to_fill, Entity* entity_to_ignore)
{
	std::vector<Entity*> QT_entities;
	
	App->entitycontroller->colliderQT->FillCollisionVector(QT_entities, collider);

	for (int i = 0; i < QT_entities.size(); i++)
	{
		if (QT_entities[i] != entity_to_ignore && QT_entities[i]->ex_state !=  DESTROYED && QT_entities[i]->isActive)
			if (SDL_HasIntersection(&collider, &QT_entities[i]->collider)) list_to_fill.push_back(QT_entities[i]);
	}

}


void j1EntityController::TownHallLevelUp()
{
	for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)
	{
		if ((*it)->type == TOWN_HALL)
		{
			App->scene->town_hall_lvl++;
			((Building*)(*it))->current_sprite[App->scene->town_hall_lvl + 1];
		}
	}
}


void j1EntityController::SubstractRandomWorkers(int num)
{
	int counter = 0;
	for (std::list<worker*>::iterator it = App->scene->workers.begin(); it != App->scene->workers.end(); it++)
	{
		if ((*it)->working_at == nullptr)
		{
			(*it)->to_destroy = true;
			counter++;
			if (counter == num)
			{
				break;
			}
		}
	}
	
	
}

void j1EntityController::DeleteWorkers()
{

}

void j1EntityController::CreateWorkers(Building * target, int num)
{
	for (int i = 0; i < num; i++)
	{
		worker* tmp = new worker(target);
		App->scene->workers.push_back(tmp);
		target->workers_inside.push_front(tmp);
	}
}

bool j1EntityController::CheckInactiveWorkers(int num)
{

	if (num == 1)
	{
		for (std::list<worker*>::iterator it = App->scene->workers.begin(); it != App->scene->workers.end(); it++)
		{
			if ((*it)->working_at == nullptr)
			{
			return true;
			}
		}
		return false;
	}
	else
	{
		int counter = 0;
		for (std::list<worker*>::iterator it = App->scene->workers.begin(); it != App->scene->workers.end(); it++)
		{
			if ((*it)->working_at == nullptr)
			{
				counter++;
			}
		}
		if (counter >= num)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

worker * j1EntityController::GetInactiveWorker()
{
	for (std::list<worker*>::iterator it = App->scene->workers.begin(); it != App->scene->workers.end(); it++)
	{
		if ((*it)->working_at == nullptr)
		{
			return *it;
		}
	}
	return nullptr;

}

void j1EntityController::AssignWorker(Building * building, worker * worker)
{
	worker->working_at = building;
	building->workers_inside.push_back(worker);
}

void j1EntityController::DestroyWorkers()
{
	for (std::list<worker*>::iterator it = App->scene->workers.begin(); it != App->scene->workers.end(); it++)
	{
		if ((*it)->to_destroy)
		{
			worker* tmp = (*it);
			if (tmp->working_at != nullptr)
			{
				(*it)->working_at->workers_inside.remove(*it);
			}
			(*it)->farm->workers_inside.remove(*it);
			App->scene->workers.remove(*it);
			RELEASE(tmp);
		}
	}
}

void j1EntityController::UnassignRandomWorker()
{
	for (std::list<worker*>::iterator it = App->scene->workers.begin(); it != App->scene->workers.end(); it++)
	{
		if ((*it)->working_at != nullptr)
		{
			if ((*it)->working_at->ex_state != BEING_BUILT)
			{
				(*it)->working_at->CalculateResourceProduction();
				(*it)->working_at->workers_inside.remove(*it);
				(*it)->working_at = nullptr;
				break;
			}
		}
	}
}


bool j1EntityController::loadEntitiesDB(pugi::xml_node& data)
{
	pugi::xml_node NodeInfo;

	for (NodeInfo = data.child("Units").child("Unit"); NodeInfo; NodeInfo = NodeInfo.next_sibling("Unit")) {

		Unit* unitTemplate = new Unit();
		unitTemplate->type = (Type)NodeInfo.child("type").attribute("value").as_int(0);

		unitTemplate->texture	= App->tex->Load(NodeInfo.child("texture").attribute("value").as_string("error"));

		unitTemplate->current_HP		= unitTemplate->max_HP = NodeInfo.child("Stats").child("life").attribute("value").as_int(0);
		unitTemplate->attack			= NodeInfo.child("Stats").child("attack").attribute("value").as_int(0);
		unitTemplate->defense			= NodeInfo.child("Stats").child("defense").attribute("value").as_int(0);
		unitTemplate->piercing_atk		= NodeInfo.child("Stats").child("piercingDamage").attribute("value").as_int(0);
		unitTemplate->speed				= NodeInfo.child("Stats").child("movementSpeed").attribute("value").as_float(0.0f);
		unitTemplate->range				= NodeInfo.child("Stats").child("range").attribute("value").as_int(0);
		unitTemplate->line_of_sight		= NodeInfo.child("Stats").child("lineOfSight").attribute("value").as_int(0);
		unitTemplate->flying			= NodeInfo.child("Stats").child("flying").attribute("value").as_bool(false);
		unitTemplate->cost.wood_cost	= NodeInfo.child("Stats").child("woodCost").attribute("value").as_int(0);
		unitTemplate->cost.gold_cost	= NodeInfo.child("Stats").child("goldCost").attribute("value").as_int(0);
		unitTemplate->cost.worker_cost	= NodeInfo.child("Stats").child("workerCost").attribute("value").as_int(0);
		unitTemplate->cost.creation_time = NodeInfo.child("Stats").child("trainingTime").attribute("value").as_int(0);
		unitTemplate->squad_members		= NodeInfo.child("Stats").child("squadMembers").attribute("value").as_int(1);


		for (pugi::xml_node action = NodeInfo.child("Actions").child("action"); action; action = action.next_sibling("action"))
			unitTemplate->available_actions.push_back(action.attribute("id").as_uint(0));

		int size_x = NodeInfo.child("Stats").child("size").attribute("x").as_int(1);
		int size_y = NodeInfo.child("Stats").child("size").attribute("y").as_int(1);

		pugi::xml_node info = NodeInfo.child("Info");
		if (info)
		{
			InfoData* infoData = new InfoData();
			infoData->title = info.attribute("title").as_string();
			infoData->linesData.push_back(new InfoLineData(STAT, "Damage:", unitTemplate->attack));
			infoData->linesData.push_back(new InfoLineData(STAT, "Armor:", unitTemplate->defense));
			infoData->linesData.push_back(new InfoLineData(STAT, "Sight:", unitTemplate->line_of_sight));
			infoData->linesData.push_back(new InfoLineData(STAT, "Range:", unitTemplate->range));
			infoData->linesData.push_back(new InfoLineData(STAT, "Speed:", unitTemplate->speed));
			unitTemplate->infoData = infoData;
		}
		

		//TODO: https://github.com/LazyFoxStudio/Project-2/issues/13
		unitTemplate->collider = { 0,0, App->map->data.tile_width * size_x, App->map->data.tile_height * size_y };

		if (NodeInfo.child("iconData"))
			App->gui->AddIconDataUnit(unitTemplate->type, NodeInfo.child("iconData"));

		int anim_width = NodeInfo.child("Animations").child("width").attribute("value").as_int(0);
		int anim_height = NodeInfo.child("Animations").child("height").attribute("value").as_int(0);

		for (pugi::xml_node AnimInfo = NodeInfo.child("Animations").child("Animation"); AnimInfo; AnimInfo = AnimInfo.next_sibling("Animation"))
		{
			Animation* animation = new Animation();
			if (animation->LoadAnimation(AnimInfo, anim_width, anim_height))
				unitTemplate->animations.push_back(animation);
		}

		if (!unitTemplate->animations.empty()) unitTemplate->current_anim = unitTemplate->animations.front();

		DataBase.insert(std::pair<uint, Entity*>(unitTemplate->type, unitTemplate));
	}

	for (NodeInfo = data.child("Buildings").child("Building"); NodeInfo; NodeInfo = NodeInfo.next_sibling("Building")) {

		Building* buildingTemplate = new Building();
		buildingTemplate->type = (Type)NodeInfo.child("type").attribute("value").as_int(0);

		buildingTemplate->texture = App->tex->Load(NodeInfo.child("texture").attribute("value").as_string("error"));

		buildingTemplate->current_HP = buildingTemplate->max_HP = NodeInfo.child("Stats").child("life").attribute("value").as_int(0);
		//buildingTemplate->workers_inside = NodeInfo.child("Stats").child("villagers").attribute("value").as_int(0);
		buildingTemplate->defense = NodeInfo.child("Stats").child("defense").attribute("value").as_int(0);
		buildingTemplate->cost.creation_time = NodeInfo.child("Stats").child("buildingTime").attribute("value").as_int(0);
		buildingTemplate->cost.wood_cost = NodeInfo.child("Stats").child("woodCost").attribute("value").as_int(0);
		buildingTemplate->cost.gold_cost = NodeInfo.child("Stats").child("goldCost").attribute("value").as_int(0);
		buildingTemplate->size.x = NodeInfo.child("size").attribute("x").as_int(0);
		buildingTemplate->size.y = NodeInfo.child("size").attribute("y").as_int(0);

		// Needed to blit the wood collecting area correctly
		buildingTemplate->collider.w = buildingTemplate->size.x*App->map->data.tile_width;
		buildingTemplate->collider.h = buildingTemplate->size.y*App->map->data.tile_height;

		buildingTemplate->additional_size.x = NodeInfo.child("additionalSize").attribute("x").as_int(0);
		buildingTemplate->additional_size.y = NodeInfo.child("additionalSize").attribute("y").as_int(0);

		for (pugi::xml_node action = NodeInfo.child("Actions").child("action"); action; action = action.next_sibling("action"))
			buildingTemplate->available_actions.push_back(action.attribute("id").as_uint());

		pugi::xml_node info = NodeInfo.child("Info");
		if (info)
		{
			InfoData* infoData = new InfoData();
			infoData->title = info.attribute("title").as_string();
			for (pugi::xml_node line = info.child("line"); line; line = line.next_sibling("line"))
			{
				infoData->linesData.push_back(new InfoLineData(INFO, line.attribute("text").as_string()));
			}
			buildingTemplate->infoData = infoData;
		}

		pugi::xml_node IconData;
		if (NodeInfo.child("iconData"))
			App->gui->AddIconDataBuilding(buildingTemplate->type, NodeInfo.child("iconData"));

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

		DataBase.insert(std::pair<uint, Building*>(buildingTemplate->type, buildingTemplate));
	}

	return true;
}

void j1EntityController::DeleteDB()
{
	for (std::map<uint, Entity*>::iterator it = DataBase.begin(); it != DataBase.end(); it++)
	{
		Entity* entity = (*it).second;
		if (entity->IsUnit())
		{
			Unit* unit_to_delete = (Unit*)entity;
			RELEASE(unit_to_delete);
		}
		else if (entity->IsBuilding())
		{
			Building* building_to_delete = (Building*)entity;
			RELEASE(building_to_delete);
		}
	}

	DataBase.clear();
}

bool j1EntityController::CreateForest(MapLayer* trees)
{
	bool ret = true;
	for (int j = 0; j < trees->height; j++)
	{
		for (int i = 0; i < trees->width; i++)
		{
			if (trees->GetID(i, j) == 112)//found a core!
			{
				//start bfs
				Forest f;
				std::list<iPoint>borders_to_check;
				iPoint p = { i, j};//32 is hardcoded
				borders_to_check.push_back(p);
				
				while (borders_to_check.size() != 0)
				{
					iPoint curr = borders_to_check.front();
					borders_to_check.remove(curr);
					iPoint neighbors[4];
					neighbors[0].create(curr.x + 1, curr.y + 0);
					neighbors[1].create(curr.x + 0, curr.y + 1);
					neighbors[2].create(curr.x - 1, curr.y + 0);
					neighbors[3].create(curr.x + 0, curr.y - 1);

					for (uint i = 0; i < 4; ++i)
					{
						if (!App->pathfinding->IsWalkable(neighbors[i]))
						{
							//add to the list of trees and to the borders
							iPoint p = neighbors[i];
							borders_to_check.push_back(p);
							f.trees.push_back({p.x*32,p.y*32});
						}
					}
					borders_to_check.remove(curr);
				}
				forests.push_back(&f);
			}
		}
	}
	return ret;
}