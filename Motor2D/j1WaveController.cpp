#include "j1WaveController.h"
#include "j1EntityController.h"
#include "Entity.h"
#include "Squad.h"
#include "Command.h"
#include "j1Pathfinding.h"
#include "j1Map.h"
#include "j1Gui.h"
#include "j1UIScene.h"
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

	for (pugi::xml_node spawn = config.child("Spawns").child("spawn"); spawn; spawn = spawn.next_sibling("spawn"))
		spawns.push_back(fPoint(spawn.attribute("x").as_float(0), spawn.attribute("y").as_float(0)));

	return true;
}

bool j1WaveController::Start()
{
	bool ret = true;

	iPoint TownHall_pos = TOWN_HALL_POS;
	TownHall_pos = App->map->WorldToMap(TownHall_pos.x, TownHall_pos.y);
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
	iPoint TownHall_pos = TOWN_HALL_POS;
	TownHall_pos = App->map->WorldToMap(TownHall_pos.x, TownHall_pos.y);
	TownHall_pos = App->pathfinding->FirstWalkableAdjacent(TownHall_pos);
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
	BROFILER_CATEGORY("WaveController postupdate", Profiler::Color::Maroon);
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
	spawns.clear();
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
		int enemy = rand() % ((AXE_THROWER - GRUNT) + 1);    //   (last_enemy_type - first_enemy_type)
		int position = rand() % spawns.size();

		next_wave.push_back(new NextWave((Type)(GRUNT + enemy), spawns[position]));
	}

	App->gui->newWave();
}

void j1WaveController::Generate_Wave()
{
	for (std::list<NextWave*>::iterator it = next_wave.begin(); it != next_wave.end(); it++)
	{
		Squad* squad = App->entitycontroller->AddSquad((*it)->type, (*it)->spawn);
		//minimap_
		App->uiscene->minimap->AddAlert((*it)->spawn.x, (*it)->spawn.y, 5, alert_type::DANGER);


		AttackingMoveToSquad* new_atk_order = new AttackingMoveToSquad(squad->getCommander(), TOWN_HALL_POS);
		new_atk_order->flow_field = flow_field;
		squad->commands.push_back(new_atk_order);
	}

	Generate_Next_Wave();
}

void j1WaveController::Restart_Wave_Sys()
{
	current_wave = 0;
	Generate_Next_Wave();
	wave_timer.Start();

}