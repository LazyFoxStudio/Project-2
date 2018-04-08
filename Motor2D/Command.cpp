#include "Command.h"
#include "Unit.h"
#include "Squad.h"
#include "p2Log.h"
#include "j1Pathfinding.h"
#include "j1EntityController.h"
#include "j1Map.h"

// BASE CLASSES: =========================

void Command::Execute(float dt)
{
	bool ret = false;

	switch (state)    
	{
	case TO_INIT: state = UPDATE;	ret = OnInit();			break;    // state change MUST happen before calling the method
	case UPDATE:					ret = OnUpdate(dt);		break;
	case TO_STOP: state = FINISHED;	ret = OnStop();			break;
	default:												break;
	}

	if (!ret) LOG("Error in command flow");
}

void Command::Stop()	{ state = TO_STOP; }
void Command::Restart() { OnStop(); state = TO_INIT; }


//		UNITS: =============================
// MOVETO

bool MoveTo::OnInit()
{
	map_p = App->map->WorldToMap(unit->position.x, unit->position.y);
	if (!flow_field)
	{
		flow_field = App->pathfinding->CreateFlowField(map_p, dest);
		unique_field = true;

		if (!flow_field) Stop();
	}

	return true;
}

bool MoveTo::OnUpdate(float dt)
{
	map_p = App->map->WorldToMap(unit->position.x, unit->position.y);

	if (map_p.DistanceTo(dest) < PROXIMITY_FACTOR || !flow_field->getNodeAt(map_p)->parent)
		Stop();
	else
		unit->next_step += ((flow_field->getNodeAt(map_p)->parent->position - map_p).Normalized() * STEERING_FACTOR);

	return true; 
}


bool MoveTo::OnStop()
{
	if (flow_field && unique_field) delete flow_field;
	unit->next_step = { 0,0 };

	return true;
}

// ATTACKING MOVE TO

bool AttackingMoveTo::OnUpdate(float dt) 
{
	Unit* enemy = App->entitycontroller->getNearestEnemyUnit(unit->position, unit->IsEnemy());
	if (enemy)
	{
		if (unit->position.DistanceTo(enemy->position) < unit->line_of_sight)
		{
			Attack* new_attack_command = new Attack(unit);
			unit->commands.push_front(new_attack_command);
			return true;
		}
	}

	map_p = App->map->WorldToMap(unit->position.x, unit->position.y);

	if (map_p.DistanceTo(dest) < PROXIMITY_FACTOR || !flow_field->getNodeAt(map_p)->parent) 
		Stop();
	else
		unit->next_step += ((flow_field->getNodeAt(map_p)->parent->position - map_p).Normalized() * STEERING_FACTOR);

	return true;
}


// ATTACK

bool Attack::OnInit()
{
	if (!flow_field)
	{
		flow_field = new FlowField(App->map->data.width, App->map->data.height);
		unique_field = true;
	}

	return true;
}

bool Attack::OnUpdate(float dt)
{
	Unit* enemy = App->entitycontroller->getNearestEnemyUnit(unit->position, unit->IsEnemy());
	if (enemy)
	{
		if (enemy->position.DistanceTo(unit->position) < unit->range)
		{
			unit->next_step.SetToZero();
			if (!timer) timer = new j1Timer();
			else if (timer->ReadSec() > 0.5f)
			{
				enemy->current_HP -= unit->piercing_atk + (MAX(unit->attack - enemy->defense, 0));
				RELEASE(timer);
			}
		}
		else if((unit->squad ? unit->squad->isInSquadSight(enemy->position) : enemy->position.DistanceTo(unit->position) < unit->line_of_sight))
		{
			RELEASE(timer);
			map_p = App->map->WorldToMap(unit->position.x, unit->position.y);

			if (!flow_field->getNodeAt(map_p)->parent)
			{
				iPoint enemy_map_p = App->map->WorldToMap(enemy->position.x, enemy->position.y);

				if (App->pathfinding->CreatePath(map_p, enemy_map_p) < 0)		
					{ Stop(); return true; }
				else
					flow_field->updateFromPath(*App->pathfinding->GetLastPath());
			}
			else
				unit->next_step += ((flow_field->getNodeAt(map_p)->parent->position - map_p).Normalized() * STEERING_FACTOR);
		}
		else 
			Stop();
	}
	else 
		Stop();
	
	return true;
}

bool Attack::OnStop()
{
	if (flow_field && unique_field) delete flow_field;
	RELEASE(timer);
	unit->next_step = { 0,0 };
	return true;
}

// HOLD

bool Hold::OnInit()
{
	iPoint pos = App->map->WorldToMap(unit->position.x, unit->position.y);

	if (pos != held_position)
	{
		MoveTo* new_moveto_command = new MoveTo(unit, held_position);
		unit->commands.push_front(new_moveto_command);

		Restart();
	}
	return true;
}

bool Hold::OnUpdate(float dt)
{
	Unit* enemy = nullptr; // unit->SearchNearestEnemy();
	if (enemy)
	{
		iPoint enemy_position = App->map->WorldToMap(enemy->position.x, enemy->position.y);
		iPoint pos = App->map->WorldToMap(unit->position.x, unit->position.y);

		if (pos.DistanceTo(enemy_position) < unit->line_of_sight)
		{
			Attack* new_attack_command = new Attack(unit);
			new_attack_command->state = UPDATE;
			unit->commands.push_front(new_attack_command);

			Restart();
		}
	}
	
	return true;
}


// PATROL

bool Patrol::OnInit()
{
	// TODO  (Check if the paths between points are possible: here or before creating the Patrol command?
	return true;
}

bool Patrol::OnUpdate(float dt) 
{
	current_point++;
	if (current_point == patrol_points.size()) current_point = 0;

	AttackingMoveTo* new_a_moveto_command = new AttackingMoveTo(unit, patrol_points[current_point]);
	unit->commands.push_front(new_a_moveto_command);

	return true;
}


//		SQUADS: =============================
//// MOVETOSQUAD

bool MoveToSquad::OnInit()
{
	if (!unit->squad) { Stop(); return true; }
	else squad = unit->squad;
	
	squad->commander = squad->getClosestUnitTo(dest);
	if(!squad->commander) { Stop(); return true; }

	iPoint commander_map_p = App->map->WorldToMap(squad->commander->position.x, squad->commander->position.y);
	flow_field = App->pathfinding->CreateFlowField(commander_map_p, dest);

	if(!flow_field)  Stop();
	else
	{
		for (int i = 0; i < squad->units.size(); i++)
		{
			MoveTo* new_move_order = new MoveTo(squad->units[i], dest);
			new_move_order->flow_field = flow_field;
			squad->units[i]->commands.push_back(new_move_order);
		}
	}
	
	return true;
}

bool MoveToSquad::OnUpdate(float dt)
{
	bool all_idle = true;

	for (int i = 0; i < squad->units.size(); i++)
		if (!squad->units[i]->commands.empty()) all_idle = false;
	
	if (all_idle) Stop(); 

	return true;
}

bool MoveToSquad::OnStop()
{
	if (flow_field) delete flow_field;
	return true;
}

bool AttackingMoveToSquad::OnUpdate(float dt)
{
	for (int i = 0; i < squad->units.size(); i++)
	{
		Unit* current_unit = squad->units[i];
		Unit* enemy = App->entitycontroller->getNearestEnemyUnit(current_unit->position, unit->IsEnemy());
		if (enemy)
		{
			if (squad->isInSquadSight(enemy->position))
			{
				for (int j = 0; j < squad->units.size(); j++)
				{
					if (squad->units[j]->commands.empty() ? true : squad->units[j]->commands.front()->type != ATTACK)
						squad->units[j]->commands.push_front(new Attack(squad->units[j]));
				}
				Restart();
				return true;
			}
		}
	}

	bool all_idle = true;

	for (int i = 0; i < squad->units.size(); i++)
		if (!squad->units[i]->commands.empty()) all_idle = false;

	if (all_idle) Stop();

}

