#include "j1WaveController.h"
#include "j1EntityController.h"
#include "Entity.h"
#include "Squad.h"
#include "Command.h"
#include "j1Pathfinding.h"
#include "j1Map.h"

#include <time.h>


j1WaveController::j1WaveController()
{
	name = { "wavecontroller" };
}


j1WaveController::~j1WaveController()
{
}

bool j1WaveController::Awake(pugi::xml_node &config)
{
	initial_wait = config.child("initialWait").attribute("value").as_int(0);
	wait_between_waves = config.child("waitBetweenWaves").attribute("value").as_int(0);
	spawn_1.x = config.child("spawn1").attribute("x").as_float(0);
	spawn_1.y = config.child("spawn1").attribute("y").as_float(0);
	spawn_2.x = config.child("spawn2").attribute("x").as_float(0);
	spawn_2.y = config.child("spawn2").attribute("y").as_float(0);
	spawn_3.x = config.child("spawn3").attribute("x").as_float(0);
	spawn_3.y = config.child("spawn3").attribute("y").as_float(0);
	spawn_4.x = config.child("spawn4").attribute("x").as_float(0);
	spawn_4.y = config.child("spawn4").attribute("y").as_float(0);

	return true;
}

bool j1WaveController::Start()
{
	bool ret = true;

	TownHall_pos = App->map->WorldToMap(2200, 2000);
	iPoint map_org = App->map->WorldToMap(TownHall_pos.x+64, TownHall_pos.y);
	iPoint map_dest = App->map->WorldToMap(TownHall_pos.x,TownHall_pos.y);
	flow_field=App->pathfinding->CreateFlowField(map_org, map_dest);
	
	wave_timer.Start();

	if (flow_field == nullptr)
	{
		ret = false;
	}

	return ret;
}

bool j1WaveController::Update(float dt)
{	
	if (current_wave == 0 && wave_timer.ReadSec() > initial_wait)
	{
		current_wave += 1;
		wave_timer.Start();
		GenerateWave();
	}

	else if (current_wave != 0 && wave_timer.ReadSec() > wait_between_waves)
	{
		current_wave += 1;
		wave_timer.Start();
		GenerateWave();
	}

	return true;
}

bool j1WaveController::PostUpdate()
{
	return true;
}

bool j1WaveController::CleanUp()
{
	return true;
}

bool j1WaveController::Save(pugi::xml_node &) const
{
	return true;
}

bool j1WaveController::Load(pugi::xml_node &)
{
	return true;
}

int j1WaveController::CalculateWaveScore()
{
	int ret = 0;
	ret = current_wave * 2;

	return ret;
}

void j1WaveController::GenerateWave()
{
	srand(time(NULL));
	int wave_score = CalculateWaveScore();
	
	for (int i = 0; i < wave_score; i++)
	{
		int enemy = rand() % 2 + 1;
		int position = rand() % 4 + 1;
		LOG("NUMBER: %i", enemy);
		Squad* squad = nullptr;
		if (enemy == 1)
		{
			switch (position)
			{
			case 1:
				squad = App->entitycontroller->AddSquad(GRUNT, spawn_1);
				wave.push_back(squad);
				break;
			case 2:
				squad = App->entitycontroller->AddSquad(GRUNT, spawn_2);
				wave.push_back(squad);
				break;
			case 3:
				squad = App->entitycontroller->AddSquad(GRUNT, spawn_3);
				wave.push_back(squad);
				break;
			case 4:
				squad = App->entitycontroller->AddSquad(GRUNT, spawn_4);
				wave.push_back(squad);
				break;
			}
		}

		else if (enemy == 2)
		{
			switch (position)
			{
			case 1:
				squad = App->entitycontroller->AddSquad(AXE_THROWER, spawn_1);
				wave.push_back(squad);
				break;
			case 2:
				squad = App->entitycontroller->AddSquad(AXE_THROWER, spawn_2);
				wave.push_back(squad);
				break;
			case 3:
				squad = App->entitycontroller->AddSquad(AXE_THROWER, spawn_3);
				wave.push_back(squad);
				break;
			case 4:
				squad = App->entitycontroller->AddSquad(AXE_THROWER, spawn_4);
				wave.push_back(squad);
				break;
			}
		}
	}

	for (std::list<Squad*>::iterator it = wave.begin(); it != wave.end(); it++)
	{
		//(*it)->commands.push_back(new AttackingMoveToSquad((*it)->commander, TownHall_pos));
		AttackingMoveToSquad* new_atk_order = new AttackingMoveToSquad((*it)->commander, TownHall_pos);
		new_atk_order->flow_field = flow_field;
		(*it)->commands.push_back(new_atk_order);
	}

	LOG("Hola");
}
