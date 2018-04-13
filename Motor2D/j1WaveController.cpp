#include "j1WaveController.h"
#include "j1EntityController.h"
#include "Entity.h"
#include "Squad.h"
#include "Command.h"
#include "j1Pathfinding.h"
#include "j1Map.h"
#include "j1Gui.h"
#include "UI_NextWaveWindow.h"

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

	TownHall_pos = App->map->WorldToMap(2000, 2000);
	TownHall_pos = App->pathfinding->FirstWalkableAdjacent(TownHall_pos);
	flow_field = App->pathfinding->RequestFlowField(TownHall_pos);
	
	wave_timer.Start();

	current_wave = 0;
	Generate_Next_Wave();

	return ret;
}

void j1WaveController::updateFlowField()
{
	if (flow_field_aux) flow_field_aux->finished = true;
	flow_field_aux = App->pathfinding->RequestFlowField(TownHall_pos);
}

bool j1WaveController::Update(float dt)
{	
	BROFILER_CATEGORY("Waves Update", Profiler::Color::Black);
	if (current_wave == 0 && wave_timer.ReadSec() > initial_wait && flow_field->stage == COMPLETED)
	{
		current_wave += 1;
		wave_timer.Start();
		Generate_Wave();
	}

	else if (current_wave >= 1 && wave_timer.ReadSec() > wait_between_waves)
	{
		current_wave += 1;
		wave_timer.Start();
		Generate_Wave();
	}

	return true;
}

bool j1WaveController::PostUpdate()
{
	if (flow_field_aux ? flow_field_aux->stage == COMPLETED : false)
	{
		for (int i = 0; i < flow_field->width; i++)
		{
			for (int j = 0; j < flow_field->height; j++)
				flow_field->field[i][j] = flow_field_aux->field[i][j];
		}
		flow_field_aux->finished = true;
	}
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
	ret = (current_wave+1) * 2;

	return ret;
}

void j1WaveController::Generate_Next_Wave()
{
	srand(time(NULL));
	int wave_score = CalculateWaveScore();
	next_wave.clear();
	
	for (int i = 0; i < wave_score; i++)
	{
		int enemy = rand() % 2 + 1;
		int position = rand() % 4 + 1;
		LOG("NUMBER: %i", enemy);
		NextWave* aux_squad=nullptr;
		if (enemy == 1)
		{
			switch (position)
			{
			case 1:
				/*squad = App->entitycontroller->AddSquad(GRUNT, spawn_1);
				wave.push_back(squad);*/
				aux_squad = new NextWave(GRUNT,spawn_1);
				next_wave.push_back(aux_squad);
				break;
			case 2:
				/*squad = App->entitycontroller->AddSquad(GRUNT, spawn_2);
				wave.push_back(squad);*/
				aux_squad = new NextWave(GRUNT, spawn_2);
				next_wave.push_back(aux_squad);
				break;
			case 3:
				/*squad = App->entitycontroller->AddSquad(GRUNT, spawn_3);
				wave.push_back(squad);*/
				aux_squad = new NextWave(GRUNT, spawn_3);
				next_wave.push_back(aux_squad);
				break;
			case 4:
				/*squad = App->entitycontroller->AddSquad(GRUNT, spawn_4);
				wave.push_back(squad);*/
				aux_squad = new NextWave(GRUNT, spawn_4);
				next_wave.push_back(aux_squad);
				break;
			}
		}

		else if (enemy == 2)
		{
			switch (position)
			{
			case 1:
				/*squad = App->entitycontroller->AddSquad(AXE_THROWER, spawn_1);
				wave.push_back(squad);*/
				aux_squad = new NextWave(AXE_THROWER, spawn_1);
				next_wave.push_back(aux_squad);
				break;
			case 2:
				/*squad = App->entitycontroller->AddSquad(AXE_THROWER, spawn_2);
				wave.push_back(squad);*/
				aux_squad = new NextWave(AXE_THROWER, spawn_2);
				next_wave.push_back(aux_squad);
				break;
			case 3:
				/*squad = App->entitycontroller->AddSquad(AXE_THROWER, spawn_3);
				wave.push_back(squad);*/
				aux_squad = new NextWave(AXE_THROWER, spawn_3);
				next_wave.push_back(aux_squad);
				break;
			case 4:
				/*squad = App->entitycontroller->AddSquad(AXE_THROWER, spawn_4);
				wave.push_back(squad);*/
				aux_squad = new NextWave(AXE_THROWER, spawn_4);
				next_wave.push_back(aux_squad);
				break;
			}
		}
	}

	App->gui->newWave();
}

void j1WaveController::Generate_Wave()
{
	for (std::list<NextWave*>::iterator it = next_wave.begin(); it != next_wave.end(); it++)
	{
		Squad* aux_squad = new Squad();
		aux_squad = App->entitycontroller->AddSquad((*it)->type, (*it)->spawn);

		AttackingMoveToSquad* new_atk_order = new AttackingMoveToSquad(aux_squad->commander, TownHall_pos);
		new_atk_order->flow_field = flow_field;
		aux_squad->commands.push_back(new_atk_order);
	}

	Generate_Next_Wave();
}

void j1WaveController::Restart_Wave_Sys()
{
	current_wave = 0;
	Generate_Next_Wave();
	wave_timer.Start();

}