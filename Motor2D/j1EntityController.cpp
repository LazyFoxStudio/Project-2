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
#include "j1ParticleController.h"
#include "UI_WarningMessages.h"
#include "j1WaveController.h"
#include "UI_Button.h"
#include "Building.h"
#include "Quadtree.h"
#include "UI_InfoTable.h"
#include "j1Tutorial.h"
#include "UI_CooldownsDisplay.h"
#include <algorithm>

#define SQUAD_MAX_FRAMETIME 0.1f
#define ENITITY_MAX_FRAMETIME 0.3f

#define MOUSE_RADIUS 15 //(in pixels)

j1EntityController::j1EntityController() { name = "entitycontroller"; pausable = false; }

bool j1EntityController::Start()
{
	colliderQT = new Quadtree({ 0,0,App->map->data.width*App->map->data.tile_width,App->map->data.height*App->map->data.tile_height }, 0);
	
	lose_game = App->console->AddFunction("lose_game",this,0,0);
	reset_hero_cd = App->console->AddFunction("reset_hero_cd", this, 0, 0);;
	complete_buildings = App->console->AddFunction("creation_speed", this, 1, 2, " entity , time_to_create");
	kill_selected = App->console->AddFunction("kill_selected",this,0,0);
	change_stat = App->console->AddFunction("change_stat", this, 3, 3, " entity , stat , value");
	next_wave = App->console->AddFunction("next_wave", this, 0, 0);

	new_wood_cost = App->console->AddFunction("change_wood_cost", this, 2, 2, "entity, cost");
	new_worker_cost = App->console->AddFunction("change_worker_cost", this, 2, 2, "entity, cost");
	new_gold_cost = App->console->AddFunction("change_gold_cost", this, 2, 2, "entity, cost");
	new_oil_cost = App->console->AddFunction("change_oil_cost", this, 2, 2, "entity, cost");

	spawn_squad = App->console->AddFunction("spawn_squad", this, 1, 3, "entity, x, y");
	spawn_building = App->console->AddFunction("spawn_building", this, 3, 3, "building, x, y");

	iPoint town_hall_pos = TOWN_HALL_POS;
	return true;
}

bool CompareSquad(Squad* s1, Squad* s2)
{
	return s1 == s2;
}

bool j1EntityController::Update(float dt)
{
	BROFILER_CATEGORY("Entites update", Profiler::Color::Bisque);

	if (App->input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN) { debug = !debug; App->map->debug = debug; };
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
			
			(*it)->Draw(dt);

			if (App->render->CullingCam((*it)->position))
			{
				if (debug) debugDrawEntity(*it);
			}
		}
	}

	if (App->isPaused())
		return true;


	BROFILER_CATEGORY("Squad update", Profiler::Color::BurlyWood);
	for (std::list<Squad*>::iterator it = squads.begin(); it != squads.end() && !App->scene->toRestart; it++)
	{
		if (!(*it)->Update(dt)) squads_to_destroy.push_back((*it)->UID);
	}

	if (App->scene->toRestart) return true;

	

	BROFILER_CATEGORY("Entity controller update", Profiler::Color::Maroon);

	if (to_build_type != NONE_ENTITY)
		buildingCalculations();
	else if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) != KEY_IDLE && !App->actionscontroller->doingAction_lastFrame && (hero ? hero->current_skill == 0 : true) && !App->gui->leftClickedOnUI)
		selectionControl();
	else if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_DOWN && !App->actionscontroller->doingAction_lastFrame && !App->gui->rightClickedOnUI)
	{
		commandControl();
		if (App->audio->followOrdersCooldown.ReadSec() > 5)
		{
			App->audio->followOrdersCooldown.Restart();
			HandleOrdersSFX();
		}
	}
	if ((App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN || App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT)) && to_build_type != NONE_ENTITY && App->actionscontroller->doingAction)
	{
		to_build_type = NONE_ENTITY;
		App->actionscontroller->activateAction(NO_ACTION);
	}

	if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && town_hall != nullptr && (!App->tutorial->doingTutorial || App->tutorial->allowTHSelection))
	{
		if (town_hall->isSelected == true) //center camera
		{
			App->render->camera.x = - town_hall->position.x + App->render->camera.w/2;
			App->render->camera.y = - town_hall->position.y + App->render->camera.h *0.3;
		}
		
		for (std::list<Entity*>::iterator it_e = selected_entities.begin(); it_e != selected_entities.end(); it_e++)
			(*it_e)->isSelected = false;
		selected_entities.clear();

		selected_entities.push_back(town_hall);
		if (App->tutorial->doingTutorial)
			App->tutorial->taskCompleted(SELECT_TOWN_HALL);
		town_hall->isSelected = true;
		App->gui->newSelectionDone();
		
	}

	if (App->input->GetKey(SDL_SCANCODE_TAB) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN && hero != nullptr && (!App->tutorial->doingTutorial || App->tutorial->allowHeroSelection))
	{
		if (hero->isSelected == true) //center camera
		{
			App->render->camera.x = -hero->position.x + App->render->camera.w / 2;
			App->render->camera.y = -hero->position.y + App->render->camera.h * 0.4;
		}
		for (std::list<Entity*>::iterator it_e = selected_entities.begin(); it_e != selected_entities.end(); it_e++)
		{
			(*it_e)->isSelected = false;
		}
		selected_entities.clear();
		selected_squads.clear();

		selected_entities.push_back(hero);
		if (App->tutorial->doingTutorial)
			App->tutorial->taskCompleted(SELECT_HERO);
		selected_squads.push_back(hero->squad);
		hero->isSelected = true;
	
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
				if (to_build_type == LUMBER_MILL && !App->tutorial->doingTutorial)
				{
					//Hardcoded
					Button* barracks = App->gui->GetActionButton(5);
					barracks->Unlock();					
					Button* farms = App->gui->GetActionButton(7);
					farms->Unlock();
					Button* mine = App->gui->GetActionButton(22);
					mine->Unlock();
					Button* turret = App->gui->GetActionButton(23);
					turret->Unlock();
					Button* hut = App->gui->GetActionButton(24);
					hut->Unlock();
					Button* church = App->gui->GetActionButton(25);
					church->Unlock();
					Button* blacksmith = App->gui->GetActionButton(26);
					blacksmith->Unlock();
				}

				App->gui->warningMessages->hideMessage(NO_TREES);
				App->entitycontroller->SpendCost(to_build_type);

				if ((App->input->GetKey(SDL_SCANCODE_LSHIFT) != KEY_DOWN && App->input->GetKey(SDL_SCANCODE_LSHIFT) != KEY_REPEAT) || App->tutorial->doingTutorial)
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
		iPoint map_p = App->map->WorldToMap(unit->position.x, unit->position.y);
		iPoint world_p = App->map->MapToWorld(map_p.x, map_p.y);
		App->render->DrawQuad({ world_p.x, world_p.y, App->map->data.tile_width, App->map->data.tile_height }, Grey, false);
		App->render->DrawCircle(unit->position.x, unit->position.y, unit->line_of_sight, Blue);
		App->render->DrawCircle(unit->mov_target.x, unit->mov_target.y, 5, Red);


		if (unit->squad ? !unit->squad->atk_slots.empty() : false)
		{
			if (unit == unit->squad->getCommander())
			{
				for (std::list<iPoint>::iterator it = unit->squad->atk_slots.begin(); it != unit->squad->atk_slots.end(); it++)
				{
					iPoint world_p = App->map->MapToWorld((*it).x, (*it).y);
					world_p += {App->map->data.tile_width / 2, App->map->data.tile_height / 2};
					App->render->DrawCircle(world_p.x, world_p.y, 5, Blue);
				}
			}
		}
	}
	else if (entity->IsBuilding())
	{
		std::vector<iPoint> building_slots;
		Building* building = (Building*)entity;
		building->calculateAttackSlots(building_slots);

		for (int i = 0; i < building_slots.size(); i++)
		{
			iPoint world_p = App->map->MapToWorld(building_slots[i].x, building_slots[i].y);
			world_p += {App->map->data.tile_width / 2, App->map->data.tile_height / 2};
			App->render->DrawCircle(world_p.x, world_p.y, 5, Green);
		}
	}
}

SFXList j1EntityController::GetOrdersSFXFromType(Type type)
{
	switch (type)
	{
	case Type::NONE_ENTITY:
		break;
	case Type::FOOTMAN:
		return SFXList::SFX_FOOTMAN_FOLLOWING_ORDERS;
		break;
	case Type::ARCHER:
		return SFXList::SFX_ARCHER_FOLLOWING_ORDERS;
		break;
	case Type::KNIGHT:
		return SFXList::SFX_KNIGHT_FOLLOWING_ORDERS;
		break;
	case Type::GRYPHON:
		return SFXList::SFX_GRYPHON_FOLLOWING_ORDERS;
		break;
	case Type::BALLISTA:
		//App->audio->PlayFx(SFXList::SFX_BALLISTA_READY, volume);
		break;
	case Type::FLYING_MACHINE:
		return SFXList::SFX_FLYING_MACHINE_FOLLOWING_ORDERS;
		break;
	default:
		break;
	}
}

void j1EntityController::HandleAttackSFX(Type type, int volume)
{
	switch (type)
	{
	case Type::NONE_ENTITY:
		break;
	case Type::HERO_1:
		App->audio->PlayFx(SFX_HERO_YHAMAM_BASICATTACK, volume);
		break;
	case Type::FOOTMAN:
		App->audio->PlayFx(SFX_MISCELLANEOUS_SWORD_CLASH, volume);
		break;
	case Type::ARCHER:
		App->audio->PlayFx(SFX_MISCELLANEOUS_ARROW, volume);
		break;
	case Type::KNIGHT:
		App->audio->PlayFx(SFX_MISCELLANEOUS_SWORD_CLASH, volume);
		break;
	case Type::GRYPHON:
		App->audio->PlayFx(SFX_MISCELLANEOUS_GRYPHON, volume);
		break;
	case Type::BALLISTA:
		App->audio->PlayFx(SFX_MISCELLANEOUS_BALLISTA, volume);
		break;
	case Type::FLYING_MACHINE:
		App->audio->PlayFx(SFX_MISCELLANEOUS_FLYINGMACHINE, volume);
		break;
	case Type::GRUNT:
		App->audio->PlayFx(SFX_MISCELLANEOUS_SWORD_CLASH, volume);
		break;
	case Type::AXE_THROWER:
		App->audio->PlayFx(SFX_MISCELLANEOUS_AXETHROW, volume);
		break;
	case Type::DEATH_KNIGHT:
		App->audio->PlayFx(SFX_MISCELLANEOUS_DEATH_KNIGHT, volume);
		break;
	case Type::DRAGON:
		App->audio->PlayFx(SFX_MISCELLANEOUS_DRAGON, volume);
		break;
	case Type::CATAPULT:
		App->audio->PlayFx(SFX_MISCELLANEOUS_CATAPULT, volume);
		break;
	case Type::JUGGERNAUT:
		App->audio->PlayFx(SFX_MISCELLANEOUS_JUGGERNAUT, volume);
		break;
	default:
		break;
	}
}
void j1EntityController::HandleReadySFX(Type type, int volume)
{
	switch (type)
	{
	case Type::NONE_ENTITY:
		break;
	case Type::FOOTMAN:
		App->audio->PlayFx(SFXList::SFX_FOOTMAN_READY, volume);
		break;
	case Type::ARCHER:
		App->audio->PlayFx(SFXList::SFX_ARCHER_READY, volume);
		break;
	case Type::KNIGHT:
		App->audio->PlayFx(SFXList::SFX_KNIGHT_READY, volume);
		break;
	case Type::GRYPHON:
		App->audio->PlayFx(SFXList::SFX_GRYPHON_READY, volume);
		break;
	case Type::BALLISTA:
		//App->audio->PlayFx(SFXList::SFX_BALLISTA_READY, volume);
		break;
	case Type::FLYING_MACHINE:
		App->audio->PlayFx(SFXList::SFX_FLYING_MACHINE_READY, volume);
		break;
	default:
		break;
	}
}
void j1EntityController::HandleOrdersSFX()
{
	for (std::list<Squad*>::iterator it = selected_squads.begin(); it != selected_squads.end(); it++)
	{
		App->audio->PlayFx((*it)->FollowingOrdersSFX, 70);
	}
}
void j1EntityController::HandleParticles(Type type, fPoint pos, fPoint obj, float speed)
{
	switch (type)
	{
	case NONE_ENTITY:
		break;
	case HERO_1:
		App->particle->AddProjectile(particleType::PYAHMAM_AA, pos, obj, speed);
		break;
	case ARCHER:
		App->particle->AddProjectile(particleType::PARROW, pos, obj, speed);
		break;
	case BALLISTA:
		App->particle->AddProjectile(particleType::PBALLISTA, pos, obj, speed);
		break;
	case Type::FLYING_MACHINE:
		App->particle->AddProjectile(particleType::PFLYINGMACHINE, pos, obj, speed);
		break;
	case AXE_THROWER:
		App->particle->AddProjectile(particleType::PTOMAHAWK, pos, obj, speed);
		break;
	case Type::DEATH_KNIGHT:
		App->particle->AddProjectile(particleType::PDEATHKNIGHT, pos, obj, speed);
		break;
	case Type::DRAGON:
		//App->particle->AddProjectile(particleType::PDRAGON, pos, obj, speed);
		break;
	case Type::CATAPULT:
		App->particle->AddProjectile(particleType::PCATAPULT, pos, obj, speed);
		break;
	case Type::JUGGERNAUT:
		App->particle->AddProjectile(particleType::PJUGGERNAUT, pos, obj, speed);
		break;
	default:
		break;
	}
}
void j1EntityController::GetTotalIncome()
{
	App->scene->wood_production_per_second = 0;
	App->scene->gold_production_per_second = 0;
	for (std::list<Entity*>::iterator tmp = entities.begin(); tmp != entities.end(); tmp++)
	{
		if ((*tmp)->IsBuilding())
		{
			if ((*tmp)->type == LUMBER_MILL && (*tmp)->ex_state != DESTROYED)
				App->scene->wood_production_per_second += ((Building*)(*tmp))->resource_production;
			else if ((*tmp)->type == MINE && (*tmp)->ex_state != DESTROYED)
				App->scene->gold_production_per_second += ((Building*)(*tmp))->resource_production;
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
	//SAVE_UNITS
	pugi::xml_node units_node = file.append_child("Units");

	pugi::xml_node hero_node = units_node.append_child("Hero");
	Hero* heroE = (Hero*)App->entitycontroller->getEntitybyID(App->entitycontroller->hero_UID);
	hero_node.append_attribute("hp") = heroE->current_HP;
	hero_node.append_attribute("x") = heroE->position.x;
	hero_node.append_attribute("y") = heroE->position.y;

	for (std::list<Squad*>::const_iterator it = squads.begin(); it != squads.end(); it++)
	{
		std::vector<Unit*> units_of_squad;
		(*it)->getUnits(units_of_squad);
		if (units_of_squad.data()[0]->type != HERO_2 && units_of_squad.data()[0]->type != HERO_1)
		{
			pugi::xml_node squad_node = units_node.append_child("squad");
			squad_node.append_attribute("type_enum") = units_of_squad.data()[0]->type;//SOMEWHAT UNSTABLE

			//READABLE UNITS
			switch (units_of_squad.data()[0]->type)
			{
			case FOOTMAN:
				squad_node.append_attribute("type") = "FOOTMAN";
				break;
			case ARCHER:
				squad_node.append_attribute("type") = "ARCHER";
				break;
			case KNIGHT:
				squad_node.append_attribute("type") = "KNIGHT";
				break;
			case GRYPHON:
				squad_node.append_attribute("type") = "GRYPHON";
				break;
			case BALLISTA:
				squad_node.append_attribute("type") = "BALLISTA";
				break;
			case FLYING_MACHINE:
				squad_node.append_attribute("type") = "FLYING_MACHINE";
				break;
			case GRUNT:
				squad_node.append_attribute("type") = "GRUNT";
				break;
			case AXE_THROWER:
				squad_node.append_attribute("type") = "AXE_THROWER";
				break;
			case DEATH_KNIGHT:
				squad_node.append_attribute("type") = "DEATH_KNIGHT";
				break;
			case DRAGON:
				squad_node.append_attribute("type") = "DRAGON";
				break;
			case CATAPULT:
				squad_node.append_attribute("type") = "CATAPULT";
				break;
			case JUGGERNAUT:
				squad_node.append_attribute("type") = "JUGGERNAUT";
				break;
			default:
				squad_node.append_attribute("type") = "UNKNOWN UNIT";
				break;
			}//LEGIBLE TEXT FOR WHAT TYPE OF UNIT IT IS

			for (int i = 0; i < units_of_squad.size(); ++i)
			{
				pugi::xml_node unit_node = squad_node.append_child("unit");
				unit_node.append_attribute("hp") = units_of_squad.data()[i]->current_HP;
				unit_node.append_attribute("pos_x") = units_of_squad.data()[i]->position.x;
				unit_node.append_attribute("pos_y") = units_of_squad.data()[i]->position.y;

				//:^)
				if (units_of_squad.data()[i]->effects.size() > 0)
				{
					for (std::list<Effect*>::const_iterator ef = units_of_squad.data()[i]->effects.begin(); ef != units_of_squad.data()[i]->effects.end(); ef++)
					{
						pugi::xml_node effect_node = unit_node.append_child("effect");
						int new_dur = (*ef)->duration - (*ef)->timer.ReadSec();
						effect_node.append_attribute("duration_left") = new_dur;
						effect_node.append_attribute("stat") = (*ef)->type;
						effect_node.append_attribute("sign") = (*ef)->sign;
						effect_node.append_attribute("amount") = (*ef)->buff;
					}
				}
			}
		}
	}

	//SAVE_BUILDINGS
	pugi::xml_node buildings_node = file.append_child("Buildings");

	pugi::xml_node tw = buildings_node.append_child("town_hall");
	tw.append_attribute("hp") = town_hall->current_HP;

	for (std::list<Entity*>::const_iterator it = entities.begin(); it != entities.end(); it++)
	{
		if ((*it)->IsBuilding() && (*it)->type != TOWN_HALL)
		{
			pugi::xml_node building_node = buildings_node.append_child("Building");
			building_node.append_attribute("hp") = (*it)->current_HP;
			building_node.append_attribute("pos_x") = (*it)->position.x;
			building_node.append_attribute("pos_y") = (*it)->position.y;
			building_node.append_attribute("type_enum") = (*it)->type;
			switch ((*it)->type)
			{
			case BARRACKS:
				building_node.append_attribute("type") = "BARRACKS";
				break;
			case LUMBER_MILL:
				building_node.append_attribute("type") = "LUMBER_MILL";
				break;
			case FARM:
				building_node.append_attribute("type") = "FARM";
				break;
			case MINE:
				building_node.append_attribute("type") = "MINE";
				break;
			case TURRET:
				building_node.append_attribute("type") = "TURRET";
				break;
			case GNOME_HUT:
				building_node.append_attribute("type") = "GNOME_HUT";
				break;
			case BLACKSMITH:
				building_node.append_attribute("type") = "BLACKSMITH";
				break;
			default:
				building_node.append_attribute("type") = "UNKNOWN BUILDING";
				break;
			}
		}
	}

	return true;
}

bool j1EntityController::Load(pugi::xml_node& file)
{
	//UNITS

	for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)
	{
		if ((*it)->type != HERO_1 && (*it)->type != HERO_2 && (*it)->type != TOWN_HALL)
		{
			if ((*it)->IsUnit())
			{
				((Unit*)(*it))->commands.clear();
			}
			App->gui->entityDeleted(*it);
			DeleteEntity((*it)->UID);
		}
	}
	pugi::xml_node units = file.child("Units");
	pugi::xml_node squads_node;

	for (squads_node = units.child("squad"); squads_node; squads_node = squads_node.next_sibling("squad"))
	{
		Type type = (Type)squads_node.attribute("type_enum").as_int();

		Squad* squad = AddSquad(type, { 1000,1000 });//WILL NEED CHANGE :^)
		std::vector<Unit*> units_of_squad;
		squad->getUnits(units_of_squad);
		pugi::xml_node node_units = squads_node.child("unit");
		int i = -1;
		for (i = 0; node_units; ++i)
		{
			units_of_squad.data()[i]->current_HP = node_units.attribute("hp").as_int();
			units_of_squad.data()[i]->position.x = node_units.attribute("pos_x").as_int();
			units_of_squad.data()[i]->position.y = node_units.attribute("pos_y").as_int();
			//effects
			//:^)
			if (node_units.child("effect"))
			{
				pugi::xml_node effect_node;
				for (effect_node = node_units.child("effect"); effect_node; effect_node = effect_node.next_sibling("effect"))
					{
						int duration = effect_node.attribute("duration_left").as_int();
						operation_sign sign = (operation_sign)effect_node.attribute("sign").as_int();
						stat_affected sta = (stat_affected)effect_node.attribute("stat").as_int();
						int amount = effect_node.attribute("amount").as_int();
						switch (sta)
						{
						case ATTACK_STAT:
							units_of_squad.data()[i]->AddDamagebuff(duration, amount, sign);
							break;
						case PIERCING_ATK_STAT:
							units_of_squad.data()[i]->AddPiercingDamagebuff(duration, amount, sign);
							break;
						case SPEED_STAT:
							units_of_squad.data()[i]->AddSpeedbuff(duration, amount, sign);
							break;
						case RANGE_STAT:
							units_of_squad.data()[i]->AddRangebuff(duration, amount, sign);
							break;
						case DEFENSE_STAT:
							units_of_squad.data()[i]->AddDefensebuff(duration, amount, sign);
							break;
						default:
							break;
						}
					}
			}
			node_units = node_units.next_sibling("unit");
		}

		//SET THE COMMANDS FOR ENEMIES
		if (squad->isFlying())
		{
			iPoint TownHall_pos = TOWN_HALL_POS;
			TownHall_pos = App->map->WorldToMap(TownHall_pos.x, TownHall_pos.y);
			TownHall_pos = App->pathfinding->FirstWalkableAdjacent(TownHall_pos);
			squad->commands.push_back(new AttackingMoveToSquadFlying(squad->getCommander(), TownHall_pos));
		}
		else
		{
			AttackingMoveToSquad* new_atk_order = new AttackingMoveToSquad(squad->getCommander(), TOWN_HALL_POS);
			new_atk_order->flow_field = App->wavecontroller->flow_field;
			squad->commands.push_back(new_atk_order);
		}

		//WARNING
		//KILL ALL OTHER UNITS IN SQUAD
		if (i <= units_of_squad.size()-1)
		{
			int it = units_of_squad.size() - (i + 1)/*:^)*/;
			for (int j = units_of_squad.size() - 1; j > it; --j)
			{
				//KILL IT
				//DeleteEntity(units_of_squad.data()[j]->UID);
				entities_to_destroy.push_back(units_of_squad.data()[j]->UID);
				App->gui->entityDeleted(units_of_squad.data()[j]);
			}
		}
	}
	//LOAD BUILDINGS
	pugi::xml_node buildings = file.child("Buildings");
	pugi::xml_node building_node;

	pugi::xml_node tw = buildings.child("town_hall");
	town_hall->current_HP = tw.attribute("hp").as_int();

	for (building_node = buildings.child("Building"); building_node; building_node = building_node.next_sibling("squad"))
	{
		int pos_x = building_node.attribute("pos_x").as_int();
		int pos_y = building_node.attribute("pos_y").as_int();
		int hp = building_node.attribute("hp").as_int();
		Type t = (Type)building_node.attribute("type_enum").as_int();
		Building* b = addBuilding({pos_x,pos_y},t);
		b->current_HP = hp;
	}
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
			if (building_to_remove->type != MINE)
			{
				App->map->WalkabilityArea(building_to_remove->position.x, building_to_remove->position.y, building_to_remove->size.x, building_to_remove->size.y, true);
			}

			if (App->scene->toRestart)
			{
				for (std::list<worker*>::iterator it = building_to_remove->workers_inside.begin(); it != building_to_remove->workers_inside.end(); it++)
				{
					(*it)->to_destroy = true;
				}
			}

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
	hero->mov_target = hero->position;

	hero->collider.x = pos.x - (hero->collider.w / 2);
	hero->collider.y = pos.y - (hero->collider.h / 2);

	if (type == HERO_1)
	{
		hero->skill_one = new Skill(hero, 3, 100, 300, 6, AREA);		//Icicle Crash
		hero->skill_two = new Skill(hero, 0, 400, 700, 2, NONE_RANGE);	//Overflow
		hero->skill_three = new Skill(hero, 0, 200, 250, 5, LINE);		//Dragon Breath
	}
	if (type == HERO_2)
	{
		hero->skill_one = new Skill(hero, 3, 50, 3000000, 6, PLACE);	//Consecration
		hero->skill_two = new Skill(hero, 3, 50, 700, 10, HEAL);		//Circle of Light
		hero->skill_three = new Skill(hero, 3, 75, 3000000, 1, BUFF);	//Honor of the pure
	}

	App->gui->createLifeBar(hero);

	App->gui->cooldownsDisplay->heroChoosen(hero);

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
	iPoint aux = App->map->WorldToMap(pos.x, pos.y);
	aux = App->map->MapToWorld(aux.x, aux.y);
	Building* building = new Building(aux, *getBuildingFromDB(type));
	building->UID = last_UID++;
	if (type == LUMBER_MILL)
	{
		building->workersDisplay = App->gui->createWorkersDisplay(building);
		if (App->tutorial->doingTutorial)
			App->tutorial->taskCompleted(PLACE_LUMBER_MILL);
	}
	else if (type == MINE)
	{
		building->workersDisplay = App->gui->createWorkersDisplay(building);
	}
	else if (type == BARRACKS || type == GNOME_HUT || type == CHURCH)
	{
		building->queueDisplay = App->gui->createTroopCreationQueue(building);
		if (type == BARRACKS && App->tutorial->doingTutorial)
			App->tutorial->taskCompleted(PLACE_BARRACKS);
	}
	else if (type == FARM && App->tutorial->doingTutorial)
		App->tutorial->taskCompleted(PLACE_FARM);
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

	HandleReadySFX(type, 70);

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
			new_squad->FollowingOrdersSFX = GetOrdersSFXFromType(type);
			
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

	if (App->map->WalkabilityArea(pos.x, pos.y, to_build->size.x, to_build->size.y) && collisions.empty() && SDL_HasIntersection(&building_col,&buildingArea) && to_build->type != MINE)
	{
		Building* tmp = addBuilding(pos, to_build_type);
		worker* tmp2 = GetInactiveWorker();
		
		tmp->workers_inside.push_back(tmp2);
		tmp2->working_at = tmp;
		
		App->map->WalkabilityArea(pos.x, pos.y, to_build->size.x, to_build->size.y, true,false);
		App->wavecontroller->updateFlowField();

		return true;
	}
	else if (!App->map->WalkabilityArea(pos.x, pos.y, to_build->size.x, to_build->size.y, false, false, true) && collisions.empty() && SDL_HasIntersection(&building_col, &buildingArea) && to_build->type == MINE)
	{
		Building* tmp = addBuilding(pos, to_build_type);
		worker* tmp2 = GetInactiveWorker();

		tmp->workers_inside.push_back(tmp2);
		tmp2->working_at = tmp;

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

	if (to_build_type != MINE)
	{
		if (App->map->WalkabilityArea(pos.x, pos.y, to_build->size.x, to_build->size.y) && enough_resources && SDL_HasIntersection(&building_col, &buildingArea))
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
	}
	else
	{
		if (!App->map->WalkabilityArea(pos.x, pos.y, to_build->size.x, to_build->size.y, false, false, true) && enough_resources && SDL_HasIntersection(&building_col, &buildingArea))
		{
			App->render->DrawQuad({ pos.x,pos.y,to_build->size.x*App->map->data.tile_width,to_build->size.y*App->map->data.tile_height }, Translucid_Green);
		}
		else
		{
			App->render->DrawQuad({ pos.x,pos.y,to_build->size.x*App->map->data.tile_width,to_build->size.y*App->map->data.tile_height }, Red);
		}
	}


	App->render->Blit(to_build->texture, pos.x, pos.y, &to_build->sprites[COMPLETE]);

}

void j1EntityController::HandleWorkerAssignment(bool to_assign, Building * building)
{	
	if (building)
	{
		if (building->type == LUMBER_MILL || building->type == MINE)
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
			FlowField* shared_flowfield = nullptr;
			if (App->tutorial->doingTutorial)
				App->tutorial->taskCompleted(MOVE_TROOPS);
			for (std::list<Squad*>::iterator it = selected_squads.begin(); it != selected_squads.end(); it++)
			{
				(*it)->Halt();
				if((*it)->isFlying())
					(*it)->commands.push_back(new MoveToSquadFlying((*it)->getCommander(), map_p));
				else
				{
					if (!shared_flowfield)
						shared_flowfield = App->pathfinding->RequestFlowField(map_p);

					MoveToSquad* new_order = new MoveToSquad((*it)->getCommander(), map_p);
					new_order->flow_field = shared_flowfield;
					shared_flowfield->used_by++;
					(*it)->commands.push_back(new_order);
				}
			}
		}
	}
	else
	{
		if (entity->IsEnemy() && entity->ex_state != DESTROYED && entity->isActive)    //clicked on a enemy
		{
			FlowField* shared_flowfield = nullptr;
			for (std::list<Squad*>::iterator it = selected_squads.begin(); it != selected_squads.end(); it++)
			{
				(*it)->Halt();
				if ((*it)->isFlying())
					(*it)->commands.push_back(new AttackingMoveToSquadFlying((*it)->getCommander(), map_p));
				else 
				{
					if (Unit* commander = (*it)->getCommander())
						if (commander->IsMelee() && entity->IsFlying())
							continue;

					if (!shared_flowfield)
						shared_flowfield = App->pathfinding->RequestFlowField(map_p);

					MoveToSquad* new_order = new AttackingMoveToSquad((*it)->getCommander(), map_p);
					new_order->flow_field = shared_flowfield;
					shared_flowfield->used_by++;
					(*it)->commands.push_back(new_order);
				}
			}
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
							if ((*it)->type == TOWN_HALL && App->tutorial->doingTutorial)
								App->tutorial->taskCompleted(SELECT_TOWN_HALL);
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
				if (units[i]->type == HERO_1 && App->tutorial->doingTutorial)
					App->tutorial->taskCompleted(SELECT_HERO);
				units[i]->isSelected = true;
			}
		}

		if (buildings && units)
		{
			for (std::list<Entity*>::iterator it = selected_entities.begin(); it != selected_entities.end(); it++)
				if ((*it)->IsBuilding())
				{ 
					(*it)->isSelected = false;
					selected_entities.erase(it);
					it--;
				}
		}
		else if (buildings && selected_entities.size() > 1)
		{
			Entity* first_selected = selected_entities.front();
			selected_entities.clear();
			selected_entities.push_back(first_selected);
		}

		App->gui->newSelectionDone();
			
		selection_rect = { 0,0,0,0 };
		break;
	}
}

void j1EntityController::CheckCollidingWith(SDL_Rect collider, std::vector<Entity*>& list_to_fill, Entity* entity_to_ignore)
{

	for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)
	{
		if ((*it) != entity_to_ignore && (*it)->ex_state !=  DESTROYED && (*it)->isActive)
			if (SDL_HasIntersection(&collider, &(*it)->collider)) 
				list_to_fill.push_back((*it));
	}

}

bool j1EntityController::ChechUpgradeCost(UpgradeType type) const
{
	bool ret = false;
	switch (type)
	{
	case MELEE_ATTACK_UPGRADE:
		if (m_dmg_lvl == 0 && App->scene->wood >= MELEE_1_UPGRADE_COST && App->scene->gold >= MELEE_1_UPGRADE_COST)
		{
			ret = true;
		}
		else if (m_dmg_lvl == 1 && App->scene->wood >= MELEE_2_UPGRADE_COST && App->scene->gold >= MELEE_2_UPGRADE_COST)
		{
			ret = true;
		}
		break;
	case MELEE_DEFENSE_UPGRADE:
		if (m_armor_lvl == 0 && App->scene->wood >= MELEE_1_UPGRADE_COST && App->scene->gold >= MELEE_1_UPGRADE_COST)
		{
			ret = true;
		}
		else if (m_armor_lvl == 1 && App->scene->wood >= MELEE_2_UPGRADE_COST && App->scene->gold >= MELEE_2_UPGRADE_COST)
		{
			ret = true;
		}
		break;
	case RANGED_ATTACK_UPGRADE:
		if (r_dmg_lvl == 0 && App->scene->wood >= RANGED_1_UPGRADE_COST && App->scene->gold >= RANGED_1_UPGRADE_COST)
		{
			ret = true;
		}
		else if (r_dmg_lvl == 1 && App->scene->wood >= RANGED_2_UPGRADE_COST && App->scene->gold >= RANGED_2_UPGRADE_COST)
		{
			ret = true;
		}
		break;
	case RANGED_DEFENSE_UPGRADE:
		if (r_armor_lvl == 0 && App->scene->wood >= RANGED_1_UPGRADE_COST && App->scene->gold >= RANGED_1_UPGRADE_COST)
		{
			ret = true;
		}
		else if (r_armor_lvl == 1 && App->scene->wood >= RANGED_2_UPGRADE_COST && App->scene->gold >= RANGED_2_UPGRADE_COST)
		{
			ret = true;
		}
		break;
	case FLYING_ATTACK_UPGRADE:
		if (f_dmg_lvl == 0 && App->scene->wood >= FLYING_1_UPGRADE_COST && App->scene->gold >= FLYING_1_UPGRADE_COST)
		{
			ret = true;
		}
		else if (f_dmg_lvl == 1 && App->scene->wood >= FLYING_2_UPGRADE_COST && App->scene->gold >= FLYING_2_UPGRADE_COST)
		{
			ret = true;
		}
		break;
	case FLYING_DEFENSE_UPGRADE:
		if (f_armor_lvl == 0 && App->scene->wood >= FLYING_1_UPGRADE_COST && App->scene->gold >= FLYING_1_UPGRADE_COST)
		{
			ret = true;
		}
		else if (f_armor_lvl == 1 && App->scene->wood >= FLYING_2_UPGRADE_COST && App->scene->gold >= FLYING_2_UPGRADE_COST)
		{
			ret = true;
		}
		break;
	default:
		break;
	}
	return ret;
}

void j1EntityController::SpendUpgradeResources(UpgradeType type)
{
	switch (type)
	{
	case MELEE_ATTACK_UPGRADE:
		if (m_dmg_lvl == 0)
		{
			App->scene->wood -= MELEE_1_UPGRADE_COST;
			App->scene->gold -= MELEE_1_UPGRADE_COST;
		}
		else
		{
			App->scene->wood -= MELEE_2_UPGRADE_COST;
			App->scene->gold -= MELEE_2_UPGRADE_COST;
		}
		break;
	case MELEE_DEFENSE_UPGRADE:
		if (m_armor_lvl == 0)
		{
			App->scene->wood -= MELEE_1_UPGRADE_COST;
			App->scene->gold -= MELEE_1_UPGRADE_COST;
		}
		else
		{
			App->scene->wood -= MELEE_2_UPGRADE_COST;
			App->scene->gold -= MELEE_2_UPGRADE_COST;
		}
		break;
	case RANGED_ATTACK_UPGRADE:
		if (r_dmg_lvl == 0)
		{
			App->scene->wood -= RANGED_1_UPGRADE_COST;
			App->scene->gold -= RANGED_1_UPGRADE_COST;
		}
		else
		{
			App->scene->wood -= RANGED_2_UPGRADE_COST;
			App->scene->gold -= RANGED_2_UPGRADE_COST;
		}
		break;
	case RANGED_DEFENSE_UPGRADE:
		if (r_armor_lvl == 0)
		{
			App->scene->wood -= RANGED_1_UPGRADE_COST;
			App->scene->gold -= RANGED_1_UPGRADE_COST;
		}
		else
		{
			App->scene->wood -= RANGED_2_UPGRADE_COST;
			App->scene->gold -= RANGED_2_UPGRADE_COST;
		}
		break;
	case FLYING_ATTACK_UPGRADE:
		if (f_dmg_lvl == 0)
		{
			App->scene->wood -= FLYING_1_UPGRADE_COST;
			App->scene->gold -= FLYING_1_UPGRADE_COST;
		}
		else
		{
			App->scene->wood -= FLYING_2_UPGRADE_COST;
			App->scene->gold -= FLYING_2_UPGRADE_COST;
		}
		break;
	case FLYING_DEFENSE_UPGRADE:
		if (f_armor_lvl == 0)
		{
			App->scene->wood -= FLYING_1_UPGRADE_COST;
			App->scene->gold -= FLYING_1_UPGRADE_COST;
		}
		else
		{
			App->scene->wood -= FLYING_2_UPGRADE_COST;
			App->scene->gold -= FLYING_2_UPGRADE_COST;
		}
		break;
	}
}

void j1EntityController::UpgradeUnits(UpgradeType type)
{
	switch (type)
	{
	case MELEE_ATTACK_UPGRADE:
		DataBase[FOOTMAN]->piercing_atk += ATTACK_UPGRADE_GROWTH;
		DataBase[KNIGHT]->piercing_atk += ATTACK_UPGRADE_GROWTH;
		break;
	case MELEE_DEFENSE_UPGRADE:
		DataBase[FOOTMAN]->defense += DEFENSE_UPGRADE_GROWTH;
		DataBase[KNIGHT]->defense += DEFENSE_UPGRADE_GROWTH;
		break;
	case RANGED_ATTACK_UPGRADE:
		DataBase[ARCHER]->piercing_atk += ATTACK_UPGRADE_GROWTH;
		DataBase[BALLISTA]->piercing_atk += ATTACK_UPGRADE_GROWTH;
		break;
	case RANGED_DEFENSE_UPGRADE:
		DataBase[ARCHER]->defense += DEFENSE_UPGRADE_GROWTH;
		DataBase[BALLISTA]->defense += DEFENSE_UPGRADE_GROWTH;
		break;
	case FLYING_ATTACK_UPGRADE:
		DataBase[FLYING_MACHINE]->piercing_atk += ATTACK_UPGRADE_GROWTH;
		DataBase[GRYPHON]->piercing_atk += ATTACK_UPGRADE_GROWTH;
		break;
	case FLYING_DEFENSE_UPGRADE:
		DataBase[FLYING_MACHINE]->defense += DEFENSE_UPGRADE_GROWTH;
		DataBase[GRYPHON]->defense += DEFENSE_UPGRADE_GROWTH;
		break;
	}
}

void j1EntityController::RefundResources(Type type)
{
	App->scene->gold += DataBase[type]->cost.gold_cost;
	App->scene->wood += DataBase[type]->cost.wood_cost;
}

Entity* j1EntityController::getNearestEnemy(Entity* entity, int target_squad)
{
	Entity* ret = nullptr;
	for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)
	{
		if (target_squad != -1)
		{
			if (!(*it)->IsUnit()) continue;
			else if (((Unit*)(*it))->squad->UID != target_squad) continue;
		}

		if ((*it)->IsEnemy() != entity->IsEnemy() && (*it)->isActive && (*it)->ex_state != DESTROYED && !(entity->IsMelee() && (*it)->IsFlying()))
		{
			if (!ret) ret = *it;
			else
			{
				if ((*it)->position.DistanceTo(entity->position) < ret->position.DistanceTo(entity->position))
					ret = *it;
			}
		}
	}

	return ret;

}

bool j1EntityController::getNearestEnemies(Entity* entity, int squad_target, int number, std::vector<Entity*>& list_to_fill)
{
	list_to_fill.clear();

	for (int j = 0; j < number; j++)
	{
		Entity* nearest = nullptr;
		for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)
		{
			if (squad_target != -1)
			{
				if (!(*it)->IsUnit()) continue;
				else if (((Unit*)(*it))->squad->UID != squad_target) continue;
			}

			if ((*it)->IsEnemy() != entity->IsEnemy() && (*it)->isActive && (*it)->ex_state != DESTROYED && !(entity->IsMelee() && (*it)->IsFlying()))
			{
				bool used = false;
				for (int i = 0; i < list_to_fill.size(); i++)
					if (list_to_fill[i] == *it) { used = true; break; }

				if (!used)
				{
					if (!nearest) nearest = *it;
					else
					{
						if ((*it)->position.DistanceTo(entity->position) < nearest->position.DistanceTo(entity->position))
							nearest = *it;
					}
				}
			}
		}
		if (nearest) list_to_fill.push_back(nearest);
		else break;
	}

	return !list_to_fill.empty();
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
		buildingTemplate->attack = NodeInfo.child("Stats").child("attack").attribute("value").as_int(0);
		buildingTemplate->range = NodeInfo.child("Stats").child("range").attribute("value").as_int(0);
		buildingTemplate->piercing_atk = NodeInfo.child("Stats").child("piercingDamage").attribute("value").as_int(0);
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
	return true;
}

bool j1EntityController::Console_Interaction(std::string& function, std::vector<int>& arguments)
{
	if (function == lose_game->name)
	{
		town_hall->current_HP -= 9000;
	}

	if (function == reset_hero_cd->name)
	{
		(Hero*)getEntitybyID(hero_UID);//help
	}

	if (function == change_stat->name)
	{
		if (arguments.data()[0] >= Type::FOOTMAN && arguments.data()[0] <= Type::FLYING_MACHINE)
		{
			switch (arguments.data()[1])
			{
			case 0:
				DataBase[arguments.data()[0]]->attack = arguments.data()[2];
				LOG("changed attack to %d", arguments.data()[2]);
				break;
			case 1:
				DataBase[arguments.data()[0]]->piercing_atk = arguments.data()[2];
				LOG("changed piercing_atk to %d", arguments.data()[2]);
				break;
			case 2:
				DataBase[arguments.data()[0]]->defense = arguments.data()[2];
				LOG("changed defense to %d", arguments.data()[2]);
				break;
			case 3:
				DataBase[arguments.data()[0]]->line_of_sight = arguments.data()[2];
				LOG("changed line of sight to %d", arguments.data()[2]);
				break;
			case 4:
				DataBase[arguments.data()[0]]->range = arguments.data()[2];
				LOG("changed range to %d", arguments.data()[2]);
				break;
			default:
				break;
			}
		}
	}

	if (function == new_wood_cost->name)
	{
		switch (arguments.data()[0])
		{
		case 0:
			DataBase[FOOTMAN]->cost.wood_cost = arguments.data()[1];
			break;
		case 1:
			DataBase[ARCHER]->cost.wood_cost = arguments.data()[1];
			break;
		case 2:
			DataBase[KNIGHT]->cost.wood_cost = arguments.data()[1];
			break;
		case 3:
			DataBase[GRYPHON]->cost.wood_cost = arguments.data()[1];
			break;
		case 4:
			DataBase[BALLISTA]->cost.wood_cost = arguments.data()[1];
			break;
		case 5:
			DataBase[FLYING_MACHINE]->cost.wood_cost = arguments.data()[1];
			break;
		default:
			break;
		}
	}
	if (function == new_gold_cost->name)
	{
		switch (arguments.data()[0])
		{
		case 0:
			DataBase[FOOTMAN]->cost.gold_cost = arguments.data()[1];
			break;
		case 1:
			DataBase[ARCHER]->cost.gold_cost = arguments.data()[1];
			break;
		case 2:
			DataBase[KNIGHT]->cost.gold_cost = arguments.data()[1];
			break;
		case 3:
			DataBase[GRYPHON]->cost.gold_cost = arguments.data()[1];
			break;
		case 4:
			DataBase[BALLISTA]->cost.gold_cost = arguments.data()[1];
			break;
		case 5:
			DataBase[FLYING_MACHINE]->cost.gold_cost = arguments.data()[1];
			break;
		default:
			break;
		}
	}
	if (function == new_oil_cost->name)
	{
		switch (arguments.data()[0])
		{
		case 0:
			DataBase[FOOTMAN]->cost.oil_cost = arguments.data()[1];
			break;
		case 1:
			DataBase[ARCHER]->cost.oil_cost = arguments.data()[1];
			break;
		case 2:
			DataBase[KNIGHT]->cost.oil_cost = arguments.data()[1];
			break;
		case 3:
			DataBase[GRYPHON]->cost.oil_cost = arguments.data()[1];
			break;
		case 4:
			DataBase[BALLISTA]->cost.oil_cost = arguments.data()[1];
			break;
		case 5:
			DataBase[FLYING_MACHINE]->cost.oil_cost = arguments.data()[1];
			break;
		default:
			break;
		}
	}
	if (function == new_worker_cost->name)
	{
		switch (arguments.data()[0])
		{
		case 0:
			DataBase[FOOTMAN]->cost.worker_cost = arguments.data()[1];
			break;
		case 1:
			DataBase[ARCHER]->cost.worker_cost = arguments.data()[1];
			break;
		case 2:
			DataBase[KNIGHT]->cost.worker_cost = arguments.data()[1];
			break;
		case 3:
			DataBase[GRYPHON]->cost.worker_cost = arguments.data()[1];
			break;
		case 4:
			DataBase[BALLISTA]->cost.worker_cost = arguments.data()[1];
			break;
		case 5:
			DataBase[FLYING_MACHINE]->cost.worker_cost = arguments.data()[1];
			break;
		default:
			break;
		}
	}

	if (function == kill_selected->name)
	{
		for (std::list<Entity*>::iterator it = selected_entities.begin(); it != selected_entities.end(); it++)
		{
			if ((*it)->IsUnit() && !(*it)->IsHero())
			{
				LOG("deleted %d via comand", (*it)->UID);
				DeleteEntity((*it)->UID);
			}
		}
	}

	if (function == complete_buildings->name)
	{
		if (arguments.data()[0] < Type::GRUNT)
		{
			if (arguments.size() == 2)
				DataBase[arguments.data()[0]]->cost.creation_time = arguments.data()[1];
			else if (arguments.size() == 1)
				DataBase[arguments.data()[0]]->cost.creation_time = 0;
		}
	}

	if (function == next_wave->name)
	{
		for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)
		{
			if ((*it)->type >= Type::GRUNT && (*it)->type <= Type::JUGGERNAUT)
			{
				if ((*it)->IsUnit())
				{
					((Unit*)(*it))->commands.clear();
				}
				App->gui->entityDeleted(*it);
				DeleteEntity((*it)->UID);
			}
		}
	}

	if (function == spawn_squad->name)
	{
		float x = arguments.data()[1];
		float y = arguments.data()[2];
		fPoint p = { x,y };
		AddSquad((Type)arguments.data()[0], p);
	}

	if (function == spawn_building->name)
	{
		int x= arguments.data()[1];
		int y= arguments.data()[2];
		iPoint c = { x,y };
		p2Point<int> p = c;
		addBuilding( p, (Type)arguments.data()[0]);
	}

	return true;
}
