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

#define SQUAD_MAX_FRAMETIME 0.1f
#define ENITITY_MAX_FRAMETIME 0.3f

#define MOUSE_RADIUS 15 //(in pixels)

j1EntityController::j1EntityController() { name = "entitycontroller"; }


bool j1EntityController::Start()
{
	pugi::xml_document doc;
	pugi::xml_node gameData;

	gameData = App->LoadFile(doc, "GameData.xml");

	loadEntitiesDB(gameData);

	hero = addHero(iPoint(2000, 1950), HERO_1);

	iPoint town_hall_pos = TOWN_HALL_POS;
	Building* town_hall = addBuilding(town_hall_pos, TOWN_HALL);
	App->map->WalkabilityArea(town_hall_pos.x, town_hall_pos.y, town_hall->size.x, town_hall->size.y, true, false);

	//colliderQT = new Quadtree({ 0,0,App->map->data.width*App->map->data.tile_width,App->map->data.height*App->map->data.tile_height }, 0);
/*
	entity_iterator = entities.begin();
	squad_iterator = all_squads.begin();*/
	return true;
}

bool j1EntityController::Update(float dt)
{
	BROFILER_CATEGORY("Entites update", Profiler::Color::Maroon);

	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN) { debug = !debug; App->map->debug = debug; };

	for (std::list<Squad*>::iterator it = squads.begin(); it != squads.end() && !App->scene->toRestart; it++)
	{
		if (!(*it)->Update(dt))	return false;
	}

	for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)
	{
		if ((*it)->isActive || (*it) == hero)
		{
			//colliderQT->insert(*it);

			if (!App->scene->toRestart)
			{
				if (!(*it)->Update(dt))	entities_to_destroy.push_back(*it);
			}
			
			if (App->render->CullingCam((*it)->position))
			{
				(*it)->Draw(dt);
				if (debug) debugDrawEntity(*it);
			}
		}
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

	if (!App->gui->clickedOnUI)
	{
		if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) != KEY_IDLE && !App->actionscontroller->doingAction_lastFrame && hero->current_skill == 0)
			selectionControl();
		else if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_DOWN && !App->actionscontroller->doingAction_lastFrame)
			commandControl();
	}

	if ((App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN || App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT)) && to_build_type != NONE_ENTITY && App->actionscontroller->doingAction)
	{
		to_build_type = NONE_ENTITY;
		App->actionscontroller->action_type = NO_ACTION;
		App->actionscontroller->doingAction = false;
		App->actionscontroller->doingAction_lastFrame = false;
		App->gui->warningMessages->hideMessage(NO_WORKERS);
		App->gui->warningMessages->hideMessage(NO_RESOURCES);
		App->gui->warningMessages->hideMessage(NO_TREES);
	}


	return true;
}

void j1EntityController::buildingCalculations()
{
	buildingProcessDraw();

	if ((App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN) && !App->gui->clickedOnUI)
	{
		if (App->scene->workerAvalible() && App->entitycontroller->CheckCost(to_build_type))
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

				App->scene->inactive_workers -= 1;
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
	int selected_size = selected_entities.size();

	for (std::list<Entity*>::iterator it = entities_to_destroy.begin(); it != entities_to_destroy.end() && !entities_to_destroy.empty() && (*it) != nullptr; it++)
		DeleteEntity(*it);

	for (std::list<Squad*>::iterator it = squads.begin(); it != squads.end() && !squads.empty() && (*it) != nullptr; it++)
	{
		if ((*it)->units.empty())
		{/*
			if (*squad_iterator == (*it))
				squad_iterator = all_squads.begin();*/

			selected_squads.remove(*it);
			Squad* squad = (*it);
			squads.remove(*it);

			RELEASE(squad);
		}
	}

	if(selected_size != selected_entities.size())
		App->gui->newSelectionDone();

	entities_to_destroy.clear();

	/*if(debug)
		colliderQT->BlitSection();

	colliderQT->Clear();*/

	return true;
}

bool j1EntityController::CleanUp()
{
	if (!DeleteDB()) return false;

	std::list<Entity*>::iterator it = entities.begin();
	while (it != entities.end() && !entities.empty() && (*it) != nullptr)
	{
		DeleteEntity(*it);
		it++;
	}

	for (std::list<Squad*>::iterator it = squads.begin(); it != squads.end() && !squads.empty() && (*it) != nullptr; it++)
	{
		if ((*it)->units.empty())
		{/*
			if (*squad_iterator == (*it))
				squad_iterator = all_squads.begin();*/

			selected_squads.remove(*it);
			Squad* squad = (*it);
			squads.remove(*it);

			RELEASE(squad);
		}
	}

	entities_to_destroy.clear();
	entities.clear();
	selected_entities.clear();

	squads.clear();
	selected_squads.clear();
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


void j1EntityController::DeleteEntity(Entity* entity)
{
	if (entity != nullptr)
	{/*
		if(entity == *entity_iterator)
			entity_iterator = entities.begin();*/

		entities.remove(entity);
		App->gui->entityDeleted(entity);

		selected_entities.remove(entity);

		if (entity->IsUnit())
		{
			Unit* unit_to_remove = (Unit*)(entity);
			Squad* unit_squad = unit_to_remove->squad;

			if (unit_to_remove->squad != nullptr)
				unit_squad->removeUnit(unit_to_remove);
			else
				RELEASE(unit_to_remove);
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

Unit* j1EntityController::addUnit(iPoint pos, Type type, Squad* squad)
{
	Unit* unit = new Unit(pos, *getUnitFromDB(type), squad);

	entities.push_back(unit);
	App->gui->createLifeBar(unit);
	
	// if(App->render->CullingCam(unit->position))  App->audio->PlayFx(UNIT_CREATED_FX);
	return unit;
}


Hero* j1EntityController::addHero(iPoint pos, Type type)
{
	hero = new Hero();
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

	std::vector<Unit*>aux_vector;
	aux_vector.push_back(hero);

	Squad* new_squad = new Squad(aux_vector);
	squads.push_back(new_squad);

	App->gui->createLifeBar(hero);

	entities.push_back(hero);
	return hero;
}

Building* j1EntityController::addBuilding(iPoint pos, Type type)
{
	Building* building = new Building(pos, *getBuildingFromDB(type));
	entities.push_back(building);
	App->gui->createLifeBar(building);

	if (type != TOWN_HALL)
		building->current_HP = 1;

	return building;
}

Squad* j1EntityController::AddSquad(Type type, fPoint position)
{
	std::vector<Unit*> squad_vector;
	std::vector<iPoint> positions;
	iPoint map_p = App->map->WorldToMap(position.x, position.y);
	Squad* new_squad = nullptr;
	Unit* unit_template = getUnitFromDB(type);

	if (App->pathfinding->GatherWalkableAdjacents(map_p, getUnitFromDB(type)->squad_members, positions))
	{
		for (int i = 0; i < unit_template->squad_members; ++i)
		{
			iPoint world_p = App->map->MapToWorld(positions[i].x, positions[i].y);
			squad_vector.push_back(addUnit(world_p, type));
		}
		new_squad = new Squad(squad_vector);
		if (!unit_template->IsEnemy())
		{
			App->scene->wood				-= unit_template->cost.wood_cost;
			App->scene->inactive_workers	-= unit_template->cost.worker_cost;
			App->scene->workers				-= unit_template->cost.worker_cost;
		}
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

	if (App->map->WalkabilityArea(pos.x, pos.y, to_build->size.x, to_build->size.y) && App->entitycontroller->CheckCollidingWith(building_col).empty())
	{
		addBuilding(pos, to_build_type);
		App->map->WalkabilityArea(pos.x, pos.y, to_build->size.x, to_build->size.y, true,false);
		App->wavecontroller->updateFlowField();

		return true;
	}

	return false;
}

void j1EntityController::buildingProcessDraw()
{
	iPoint pos = { 0,0 };
	App->input->GetMousePosition(pos.x, pos.y);
	pos = App->render->ScreenToWorld(pos.x, pos.y);
	pos = App->map->WorldToMap(pos.x, pos.y);
	pos = App->map->MapToWorld(pos.x, pos.y);
	Building* to_build = getBuildingFromDB(to_build_type);
	bool enough_resources = true;

	if (!App->scene->workerAvalible()) { App->gui->warningMessages->showMessage(NO_WORKERS); enough_resources = false; }
	else								App->gui->warningMessages->hideMessage(NO_WORKERS);

	if (!CheckCost(to_build_type)) { App->gui->warningMessages->showMessage(NO_RESOURCES); enough_resources = false; }
	else							App->gui->warningMessages->hideMessage(NO_RESOURCES);

	App->gui->warningMessages->hideMessage(NO_TREES);


	if (App->map->WalkabilityArea(pos.x, pos.y, to_build->size.x, to_build->size.y) && enough_resources)
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
				if (building->workers_inside < MAX_VILLAGERS_LUMBERMILL && App->scene->workerAvalible())
				{
					building->workers_inside += 1;
					App->scene->inactive_workers -= 1;
				}
				else
				{
					//send sfx
				}
			}
			else
			{
				if (building->workers_inside > 0)
				{
					building->workers_inside -= 1;
					App->scene->inactive_workers += 1;
				}
			}
			building->CalculateResourceProduction();
			GetTotalIncome();
		}
	}
}

bool j1EntityController::CheckCost(Type target)
{
	return (App->scene->wood >= DataBase[target]->cost.wood_cost && App->scene->gold >= DataBase[target]->cost.gold_cost && (!DataBase[target]->IsUnit() || App->scene->workerAvalible(DataBase[target]->cost.worker_cost)));
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
		FlowField* shared_flowfield = App->pathfinding->RequestFlowField(map_p);
		for (std::list<Squad*>::iterator it = selected_squads.begin(); it != selected_squads.end(); it++)
		{
			(*it)->Halt(); 
			MoveToSquad* new_order = new MoveToSquad((*it)->commander, map_p);
			new_order->flow_field = shared_flowfield;
			(*it)->commands.push_back(new_order);
		}
	}
	else
	{
		if (entity->IsEnemy())    //clicked on a enemy
		{
			FlowField* shared_flowfield = App->pathfinding->RequestFlowField(map_p);
			for (std::list<Squad*>::iterator it = selected_squads.begin(); it != selected_squads.end(); it++)
			{
				(*it)->Halt();
				AttackingMoveToSquad* new_order = new AttackingMoveToSquad((*it)->commander, map_p);
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

		selected_entities.clear();
		selected_squads.clear();
		hero->isSelected = false;
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
							if ((*it)->IsHero())
								hero->isSelected = true;
							if (!units) units = true;
						}
					}
					else // is building
					{
						if ((*it)->ex_state == OPERATIVE)
						{
							selected_entities.push_back(*it);
							App->actionscontroller->newSquadPos = { (*it)->position.x, (*it)->position.y + (*it)->collider.h };
							if (!buildings) buildings = true;
						}
					}
				}
			}
		}

		selected_squads.unique(CompareSquad);

		for (std::list<Squad*>::iterator it = selected_squads.begin(); it != selected_squads.end(); it++)
			for(int i = 0; i < (*it)->units.size(); i++)
				selected_entities.push_back((*it)->units[i]);
		
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

Entity* j1EntityController::getNearestEnemy(fPoint position, bool isEnemy)
{
	Entity* ret = nullptr;
	for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)
	{
		if ((*it)->IsEnemy() != isEnemy && (*it)->ex_state != DESTROYED && (*it)->isActive)
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
		buildingTemplate->workers_inside = NodeInfo.child("Stats").child("villagers").attribute("value").as_int(0);
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
