#include "Building.h"
#include "j1Render.h"
#include "j1Scene.h"
#include "j1Gui.h"
#include "Minimap.h"
#include "j1EntityController.h"
#include "j1ParticleController.h"
#include "j1Input.h"
#include "j1Scene.h"
#include "j1Audio.h"
#include "UI_Chrono.h"
#include "UI_InfoTable.h"
#include "UI_WorkersDisplay.h"
#include "UI_TroopCreationQueue.h"
#include "UI_FarmWorkersManager.h"
#include "UI_CooldownsDisplay.h"
#include "Unit.h"
#include "j1Pathfinding.h"
#include "j1WaveController.h"
#include "j1ActionsController.h"

#define TURRET_ROF 0.5f
#define RANDOM_FACTOR (1.0f - (((float)(rand() % 6)) / 10.0f))

Building::Building(iPoint pos, Building& building)
{
	texture				= building.texture;
	type				= building.type;
	cost				= building.cost;

	current_HP = max_HP	= building.max_HP;
	defense				= building.defense;
	attack				= building.attack;
	piercing_atk		= building.piercing_atk;
	range				= building.range;

	collider.x = position.x = pos.x;
	collider.y = position.y	= pos.y;

	size.x = building.size.x;
	size.y = building.size.y;

	collider.w = building.collider.w;
	collider.h = building.collider.h;

	additional_size.x = building.additional_size.x;
	additional_size.y = building.additional_size.y;

	for (int i = 0; i < 9; i++)
		available_actions = building.available_actions;

	infoData = building.infoData;

	sprites = building.sprites;

	if (type == TOWN_HALL)
	{
		ex_state = OPERATIVE;
		current_sprite = &sprites[1];
	}
	else current_sprite = &sprites[CREATION_STARTED];

	healingParticleTimer.Start();
	timer.Start();
	repair_timer.Start();
}

Building::~Building()
{
	sprites.clear();
	//Should be cleared just once with the DB
	//RELEASE(infoData);
	App->gui->deleteElement(queueDisplay);
	App->gui->deleteElement(workersDisplay);
}


bool Building::Update(float dt)
{
	if (current_HP <= 0 && ex_state != DESTROYED)   Destroy();

	switch (ex_state)
	{
	case BEING_BUILT:
		HandleConstruction();
		if (timer.ReadSec() >= cost.creation_time / 2 && current_sprite == &sprites[2])
			current_sprite = &sprites[HALF_CREATED];
		break;
	case OPERATIVE:

		if (type == LUMBER_MILL || type == MINE)
			HandleResourceProduction();
		else if (type == FARM)
			HandleWorkerProduction();
		else if (type == TURRET)
			turretBehavior();
		else if (type == BARRACKS || type == GNOME_HUT || type == CHURCH)
		{
			HandleUnitProduction();
		}
		else if (type == TOWN_HALL)
		{
			Entity* hero = App->entitycontroller->getEntitybyID(App->entitycontroller->hero_UID);

			iPoint TH_center = { (int)position.x + collider.w / 2, (int)position.y + collider.h / 2 };
			if (hero != nullptr)
			{
				if (hero->isSelected)
				{
					App->render->DrawCircle(TH_center.x, TH_center.y, 250, Green);
				}

				if (TH_center.DistanceTo(iPoint(hero->position.x, hero->position.y)) < 250 && hero->current_HP < hero->max_HP)
				{
					if (healingParticleTimer.ReadSec() > 5)
					{
						App->particle->AddParticle(PHEALINGHERO, hero->position, false);
						App->audio->PlayFx(SFX_MISCELLANEOUS_THHEAL);
						healingParticleTimer.Restart();
						hero->current_HP += 10;
					}
				}
			}
		}
		break;

	}

	recently_repaired = repair_timer.ReadSec() > REPAIR_COOLDOWN;

	return true;
}

void Building::Destroy()
{
	ex_state = DESTROYED;
	App->entitycontroller->selected_entities.remove(this);
	App->entitycontroller->operative_entities.remove(this);
	isSelected = false;
	current_sprite = &sprites[RUIN];
	if (workersDisplay != nullptr)
		workersDisplay->active = false;
	if (queueDisplay != nullptr)
		queueDisplay->active = false;

	switch (type)
	{
	case FARM:
		for (std::list<worker*>::iterator it = workers_inside.begin(); it != workers_inside.end(); it++)
		{
			(*it)->to_destroy = true;
		}
		break;
	case LUMBER_MILL:
		CalculateResourceProduction();
		App->entitycontroller->GetTotalIncome();
		
		while(workers_inside.size() > 0)
		{
			App->entitycontroller->HandleWorkerAssignment(false, this);
		}

		break;
	case TOWN_HALL:
		for (std::list<worker*>::iterator it = workers_inside.begin(); it != workers_inside.end(); it++)
		{
			(*it)->to_destroy = true;
		}
		App->audio->PlayMusic(DEFEAT_THEME, 0);
		current_sprite = &sprites[4];

		if(App->gui->Chronos)
			App->gui->Chronos->counter.PauseTimer();
		if(App->gui->cooldownsDisplay)
			App->gui->cooldownsDisplay->Reset();

		App->scene->toRestart = true;
		App->scene->Restart_timer.Start();
		App->uiscene->toggleMenu(true, GAMEOVER_MENU);
		break;
	}

	if (type != MINE)
		App->map->WalkabilityArea(position.x, position.y, size.x, size.y, true);

	if (!App->scene->toRestart)
		App->wavecontroller->updateFlowField();

	timer.Start();
	App->gui->entityDeleted(this);
}


void Building::HandleConstruction()
{
	int current_time = timer.ReadSec();
	if (current_time >= cost.creation_time)
	{
		current_HP = max_HP;
		
		if (type == FARM)
		{
			App->entitycontroller->CreateWorkers(this, 5);
		}
		if (type != LUMBER_MILL || type != MINE)
		{
			if (!workers_inside.empty())
			{
				workers_inside.back()->working_at = nullptr;
				workers_inside.pop_back();
			}
		}
		else
		{
			timer.Start();
			CalculateResourceProduction();
			App->entitycontroller->GetTotalIncome();
		}
		ex_state = OPERATIVE;
		current_sprite = &sprites[COMPLETE];
	}
	else if (current_time > last_frame_time)
	{
		last_frame_time = current_time;
		current_HP += (max_HP / cost.creation_time);
	}

}


void Building::HandleResourceProduction()
{
	if (timer.ReadSec() >= 3)
	{
		timer.Start();
		if (type == MINE)
		{
			App->scene->gold += resource_production;
		}
		else if(!App->map->WalkabilityArea(position.x - (additional_size.x * App->map->data.tile_width / 2) + collider.w / 2, (position.y - (additional_size.x * App->map->data.tile_width / 2)) + collider.h / 2, additional_size.x, additional_size.y, false, true) && type == LUMBER_MILL)
			App->scene->wood += resource_production;
	}
}

void Building::AddUnitToQueue(Type type)
{
	if (App->entitycontroller->SpendCost(type))
	{
		App->entitycontroller->SubstractRandomWorkers(App->entitycontroller->DataBase[type]->cost.worker_cost);
		unit_queue.push_back(type);
		queueDisplay->pushTroop(type);
		if (unit_queue.size() == 1)
		{
			producing_unit = true;
			timer.Start();
		}
	}
	
}

void Building::HandleUnitProduction()
{
	if (unit_queue.size() > 0)
	{
		if (timer.ReadSec() >= App->entitycontroller->DataBase[unit_queue.front()]->cost.creation_time)
		{
			fPoint newSquadPos = { position.x, position.y + collider.h };
			App->entitycontroller->AddSquad(unit_queue.front(), newSquadPos);
			unit_queue.pop_front();
			if (unit_queue.size() == 0)
			{
				producing_unit = false;
			}
			else
			{
				timer.Start();
			}
		}
	}

}

void Building::CalculateResourceProduction()
{
	if (type == LUMBER_MILL&& !App->map->WalkabilityArea(position.x - (additional_size.x * App->map->data.tile_width / 2) + collider.w / 2, (position.y - (additional_size.x * App->map->data.tile_width / 2)) + collider.h / 2, additional_size.x, additional_size.y, false, true))
	{
		float production_modifier = WOOD_PER_WORKER * (1 - (float)((workers_inside.size() - 1) * 0.05f));
		resource_production = 3 * workers_inside.size() * production_modifier;
	}
	else if (type == LUMBER_MILL)
	{
		resource_production = 0;
	}
	if (type == MINE)
	{
		float production_modifier = GOLD_PER_WORKER * (1 - (float)((workers_inside.size() - 1) * 0.05f));
		resource_production = 3 * workers_inside.size() * production_modifier;
	}
}

void Building::HandleWorkerProduction()
{
	if (workers_inside.size() < MAX_VILLAGERS_FARM && !producing_worker)
	{
		timer.Start();
		producing_worker = true;
	}
	else if (workers_inside.size() < MAX_VILLAGERS_FARM && producing_worker && timer.ReadSec()>FARM_WORKER_PRODUCTION_SECONDS)
	{
		App->entitycontroller->CreateWorkers(this, 1);
		producing_worker = false;
	}
}

void Building::RepairBuilding()
{

		App->scene->wood -= REPAIR_COST;
		current_HP = max_HP;
		recently_repaired = true;
		repair_timer.Start();
	
}

void Building::DemolishBuilding()
{
	if (type != FARM)
	{
		while (workers_inside.size() > 0)
		{
			App->entitycontroller->HandleWorkerAssignment(false, this);
		}
	}
	current_HP = 0;
}

void Building::Draw(float dt)
{
	if (App->render->CullingCam(position))
	{
		App->entitycontroller->SpriteQueue.push(this);

		//App->render->Blit(texture, position.x, position.y, current_sprite);
	}

	//minimap
	if (ex_state != DESTROYED)
		App->gui->minimap->Addpoint({ (int)position.x,(int)position.y,100,100 }, Green);
	else
		App->gui->minimap->Addpoint({ (int)position.x,(int)position.y,100,100 }, Grey);
}

void Building::turretBehavior()
{
	if (timer.ReadSec() > TURRET_ROF)
	{
		fPoint building_center = { position.x + collider.w / 2, position.y + collider.h / 2 };

		for (std::list<Entity*>::iterator it = App->entitycontroller->operative_entities.begin(); it != App->entitycontroller->operative_entities.end(); it++)
		{
			if ((*it)->IsEnemy())
			{
				if (building_center.DistanceTo((*it)->position) < range)
				{
					if (App->render->CullingCam(position))
						App->entitycontroller->HandleAttackSFX(ARCHER, 30);

					App->entitycontroller->HandleParticles(ARCHER, position, { (*it)->position.x + ((*it)->collider.w / 2), (*it)->position.y + ((*it)->collider.h / 2) });

					(*it)->current_HP -= MAX((RANDOM_FACTOR * (piercing_atk + ((((int)attack - (int)(*it)->defense) <= 0) ? 0 : attack - (*it)->defense))), 1);
					
					if ((*it)->current_HP < 0) ((Unit*)(*it))->Destroy();
					timer.Start();
					break;
				}
			}
		}
	}
}

void Building::calculateAttackSlots(std::vector<iPoint>& list_to_fill)
{
	iPoint map_p = App->map->WorldToMap(position.x, position.y);

	for(int i = map_p.x - 1; i <= map_p.x + size.x; i++)
		for (int j = map_p.y - 1; j <= map_p.y + size.y; j++)
		{
			iPoint atk_slot = { i,j };
			if (App->pathfinding->IsWalkable(atk_slot))
				list_to_fill.push_back(atk_slot);
		}
}