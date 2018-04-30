#include "Building.h"
#include "j1Render.h"
#include "j1Scene.h"
#include "j1UIScene.h"
#include "j1EntityController.h"
#include "j1Input.h"
#include "j1Scene.h"
#include "j1Audio.h"
#include "UI_Chrono.h"
#include "UI_InfoTable.h"
#include "UI_WorkersDisplay.h"

Building::Building(iPoint pos, Building& building)
{
	texture				= building.texture;
	type				= building.type;
	cost				= building.cost;

	current_HP = max_HP	= building.max_HP;
	defense				= building.defense;

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

	timer.Start();
}

Building::~Building()
{
	sprites.clear();
	//Should be cleared just once with the DB
	//RELEASE(infoData);
	App->gui->deleteElement(workersDisplay);
}


bool Building::Update(float dt)
{
	//minimap_
	if (App->uiscene->minimap) 
	{
		if(ex_state != DESTROYED)	App->uiscene->minimap->Addpoint({ (int)position.x,(int)position.y,100,100 }, Green);
		else						App->uiscene->minimap->Addpoint({ (int)position.x,(int)position.y,100,100 }, Grey);
	}

	if (current_HP <= 0 && ex_state != DESTROYED)   Destroy();

	switch (ex_state)
	{
	case BEING_BUILT:
		HandleConstruction();
		if (timer.ReadSec() >= cost.creation_time / 2 && current_sprite == &sprites[2])
			current_sprite = &sprites[HALF_CREATED];
		break;
	case OPERATIVE:

		if (type == LUMBER_MILL)
			HandleResourceProduction();
		else if (type == FARM)
			HandleWorkerProduction();
		else if (type == BARRACKS || type == GNOME_HUT || type == CHURCH)
		{
			HandleUnitProduction();
		}
		break;
		
	case DESTROYED:
		if (timer.ReadSec() > DEATH_TIME)
			App->entitycontroller->entities_to_destroy.push_back(UID);

		break;
	}

	if (recently_repaired && repair_timer.ReadSec() > REPAIR_COOLDOWN)
	{
		recently_repaired = false;
	}

	return true;
}

void Building::Destroy()
{
	ex_state = DESTROYED;
	App->entitycontroller->selected_entities.remove(this);
	current_sprite = &sprites[RUIN];


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

		break;
	case TOWN_HALL:
		for (std::list<worker*>::iterator it = workers_inside.begin(); it != workers_inside.end(); it++)
		{
			(*it)->to_destroy = true;
		}
		App->audio->PlayMusic(DEFEAT_THEME);
		current_sprite = &sprites[4];
		App->gui->Chronos->counter.PauseTimer();
		App->scene->toRestart = true;
		App->scene->Restart_timer.Start();
		App->uiscene->toggleMenu(true, GAMEOVER_MENU);
		break;
	}


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
		if (type != LUMBER_MILL)
		{
			workers_inside.back()->working_at = nullptr;
			workers_inside.pop_back();
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
		if(!App->map->WalkabilityArea(position.x - (additional_size.x * App->map->data.tile_width / 2) + collider.w / 2, (position.y - (additional_size.x * App->map->data.tile_width / 2)) + collider.h / 2, additional_size.x, additional_size.y, false, true))
			App->scene->wood += resource_production;
	}
}

void Building::AddUnitToQueue(Type type)
{
	unit_queue.push(type);
	if (unit_queue.size() == 1)
	{
		producing_unit = true;
		timer.Start();
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
			unit_queue.pop();
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
	float production_modifier = WOOD_PER_WORKER * (1 - (float)((workers_inside.size() - 1) * 0.05f));
	resource_production = 3 * workers_inside.size() * production_modifier;
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
	if (App->scene->wood >= REPAIR_COST && !recently_repaired)
	{
		App->scene->wood -= REPAIR_COST;
		current_HP = max_HP;
		recently_repaired = true;
		repair_timer.Start();
	}
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
	App->render->Blit(texture, position.x, position.y, current_sprite);
}