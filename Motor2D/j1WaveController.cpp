#include "j1WaveController.h"
#include "j1EntityController.h"
#include "Entity.h"
#include "Squad.h"
#include "Command.h"
#include "j1Pathfinding.h"
#include "j1Map.h"
#include "j1Gui.h"
#include "Minimap.h"
#include "UI_NextWaveWindow.h"
#include "j1Tutorial.h"

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

	for (pugi::xml_node spawn = config.child("Costs").child("cost"); spawn; spawn = spawn.next_sibling("cost"))
		costs.push_back(spawn.attribute("value").as_int(0));
		
	return true;
}

bool j1WaveController::Start()
{
	bool ret = true;

	iPoint TownHall_pos = TOWN_HALL_POS;
	TownHall_pos = App->map->WorldToMap(TownHall_pos.x, TownHall_pos.y);
	TownHall_pos = App->pathfinding->FirstWalkableAdjacent(TownHall_pos);
	flow_field = App->pathfinding->RequestFlowField(TownHall_pos);
	
	points = 1;

	return ret;
}

void j1WaveController::updateFlowField()
{
	if (flow_field_aux) flow_field_aux->used_by = 0;

	iPoint TownHall_pos = TOWN_HALL_POS;
	TownHall_pos = App->map->WorldToMap(TownHall_pos.x, TownHall_pos.y);
	TownHall_pos = App->pathfinding->FirstWalkableAdjacent(TownHall_pos);
	flow_field_aux = App->pathfinding->RequestFlowField(TownHall_pos);
}

void j1WaveController::forceNextWave()
{
	wave_timer.Start();
	Generate_Wave();
}

bool j1WaveController::Update(float dt)
{	
	BROFILER_CATEGORY("Waves Update", Profiler::Color::Black);
	if (current_wave == 0 && wave_timer.ReadSec() > initial_wait && flow_field->stage == COMPLETED && !App->tutorial->doingTutorial)
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

	if (App->entitycontroller->debug)
		flow_field->DebugDraw();

	return true;
}

bool j1WaveController::PostUpdate()
{
	BROFILER_CATEGORY("WaveController postupdate", Profiler::Color::Maroon);
	if (flow_field_aux)
	{
		if (flow_field_aux->stage == FAILED)
			updateFlowField();
		else if (flow_field_aux->stage == COMPLETED)
		{
			for (int i = 0; i < flow_field->width; i++)
			{
				for (int j = 0; j < flow_field->height; j++)
				{
					if (flow_field_aux->field[i][j].parent != nullptr) //TEMPORAL ?
					{
						iPoint parent_position = flow_field_aux->field[i][j].parent->position;
						flow_field->field[i][j].parent = &flow_field->field[parent_position.x][parent_position.y];
					}
				}
			}
			flow_field_aux->used_by = 0;
			flow_field_aux = nullptr;
		}
	}
	return true;
}

bool j1WaveController::CleanUp()
{
	spawns.clear();
	costs.clear();
	return true;
}

void j1WaveController::Generate_Next_Wave()
{
	srand(time(NULL));

	next_wave.clear();

	LOG("current wave %d", current_wave);
	
	if (!tutorial)
	{
		points ++;
		entity_selector(6);
	}
	else
	{
		tutorial = false;
		int position = rand() % spawns.size();
		next_wave.push_back(new NextWave((Type)(GRUNT), spawns[position]));
		current_wave = 0;
	}

	App->gui->newWave();
}

void j1WaveController::Generate_Wave()
{
	for (std::list<NextWave*>::iterator it = next_wave.begin(); it != next_wave.end(); it++)
	{
		Squad* squad = App->entitycontroller->AddSquad((*it)->type, (*it)->spawn);
		//minimap_
		App->gui->minimap->AddAlert((*it)->spawn.x, (*it)->spawn.y, 5, alert_type::DANGER);

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
			new_atk_order->flow_field = flow_field;
			squad->commands.push_back(new_atk_order);
		}
	}

	Generate_Next_Wave();
}

void j1WaveController::Restart_Wave_Sys()
{
	points = 1;
	current_wave = 0;
	Generate_Next_Wave();
	wave_timer.Start();

}

void j1WaveController::entity_selector(int num)
{
	srand(time(NULL));

	int aux_points = points;
	LOG("Points %d", points);

	while (aux_points > 0)
	{
		int enemy = rand() % (num);
		int position = rand() % spawns.size();

		if(aux_points<costs[enemy])
		{
			num-=enemy;
		}
		else
		{
			next_wave.push_back(new NextWave((Type)(GRUNT + enemy), spawns[position]));
			aux_points -= costs[enemy];
			LOG("enemy %d", enemy);
		}
	}
}

bool j1WaveController::Save(pugi::xml_node& file) const
{
	file.append_child("wave").append_attribute("value") = current_wave;
	file.append_child("points").append_attribute("value") = points;
	file.append_child("wave_timer").append_attribute("value") = wave_timer.ReadSec();

	pugi::xml_node nw = file.append_child("next_wave");
	for (std::list<NextWave*>::const_iterator it = next_wave.begin(); it != next_wave.end(); it++)
	{
		pugi::xml_node w = nw.append_child("enemies");
		switch ((*it)->type)
		{
		case GRUNT:
			w.append_attribute("type") = "GRUNT";
			break;
		case AXE_THROWER:
			w.append_attribute("type") = "AXE_THROWER";
			break;
		case DEATH_KNIGHT:
			w.append_attribute("type") = "DEATH_KNIGHT";
			break;
		case DRAGON:
			w.append_attribute("type") = "DRAGON";
			break;
		case CATAPULT:
			w.append_attribute("type") = "CATAPULT";
			break;
		case JUGGERNAUT:
			w.append_attribute("type") = "JUGGERNAUT";
			break;
		default:
			w.append_attribute("type") = "UNKNOWN ENEMY UNIT";
			break;
		}
		w.append_attribute("type_enum") = (*it)->type;
		w.append_attribute("x") = (*it)->spawn.x;
		w.append_attribute("y") = (*it)->spawn.y;
	}

	return true;
}

bool j1WaveController::Load(pugi::xml_node& file)
{
	current_wave = file.child("wave").attribute("value").as_int();
	points = file.child("points").attribute("value").as_int();
	int CHANGE_ME_IM_THE_TIMER/*:^)*/ = file.child("wave_timer").attribute("value").as_int();//readsec of the wave when closed

	for (std::list<NextWave*>::iterator it = next_wave.begin(); it != next_wave.end(); it++)
	{
		RELEASE(*it);
	}
	next_wave.clear();

	pugi::xml_node node_1 = file.child("next_wave").child("enemies");
	for (int i = 0; node_1; ++i)
	{
		Type type = (Type)node_1.attribute("type_enum").as_int();
		int x = node_1.attribute("x").as_int();
		int y = node_1.attribute("y").as_int();
		NextWave* n = new NextWave(type, fPoint(x,y));

		next_wave.push_back(n);

		node_1 = node_1.next_sibling("enemies");
	}

	return true;
}