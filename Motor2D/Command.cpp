#include "Command.h"
#include "Unit.h"
#include "Squad.h"
#include "p2Log.h"
#include "j1Pathfinding.h"
#include "j1EntityController.h"
#include "j1Map.h"
#include "j1Audio.h"
#include "j1Scene.h"
// BASE CLASSES: =========================

#define RANDOM_FACTOR (1.0f - (((float)(rand() % 6)) / 10.0f))

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
	if (!flow_field) Stop();

	return true;
}

bool MoveTo::OnUpdate(float dt)
{
	map_p = App->map->WorldToMap(unit->position.x, unit->position.y);

	if (map_p.DistanceManhattan(dest) < PROXIMITY_FACTOR || flow_field->stage == FAILED)
		Stop();
	else if(flow_field->getNodeAt(map_p)->parent)
		unit->next_step += ((flow_field->getNodeAt(map_p)->parent->position - map_p).Normalized() * STEERING_FACTOR);

	return true; 
}


bool MoveTo::OnStop()
{
	unit->next_step = { 0,0 };
	return true;
}


// ATTACK

bool Attack::OnInit()
{
	if (!enemy_positions) Stop();

	return true;
}

bool Attack::OnUpdate(float dt)
{
	map_p = App->map->WorldToMap(unit->position.x, unit->position.y);

	if (current_target.IsZero())
	{
		if (enemy_positions->empty()){
			if (!unit->squad->getEnemiesInSight(*enemy_positions)) 
				{ Stop(); return true; }
		}	
		current_target = enemy_positions->front();

		for (std::list<fPoint>::iterator it = enemy_positions->begin(); it != enemy_positions->end(); it++)
			if ((*it).DistanceTo(unit->position) < current_target.DistanceTo(unit->position))
				current_target = (*it);

		iPoint targetMap_p = App->map->WorldToMap(current_target.x, current_target.y);
		if (!App->pathfinding->CreatePath(map_p, targetMap_p) >= 0) path = *App->pathfinding->GetLastPath();
		else { Stop(); return true; }

		type = ATTACKING_MOVETO;
		return true;
	}

	Entity* enemy = App->entitycontroller->getNearestEnemy(unit->position, unit->IsEnemy());
	if(!enemy)  { Stop(); return true; }

	SDL_Rect r = { unit->position.x - unit->range, unit->position.y - unit->range, unit->range * 2, unit->range * 2};

	if(SDL_HasIntersection(&r, &enemy->collider) && enemy->isActive)
	{
		if (type == ATTACKING_MOVETO) 
			{ type = ATTACK; timer.Start(); }
		else if (timer.ReadSec() > 0.5f)
		{ 
			Unit* enemy_unit = (Unit*)enemy;
			Building* enemy_building = (Building*)enemy;
			switch (enemy->entity_type)
			{
			case UNIT:

				App->entitycontroller->HandleSFX(unit->type, 30);
				enemy_unit = (Unit*)enemy;
				enemy_unit->current_HP -= MAX((RANDOM_FACTOR * (unit->piercing_atk + ((((int)unit->attack - (int)enemy_unit->defense) <= 0) ? 0 : unit->attack - enemy_unit->defense))), 1); //dmg

				if (enemy_unit->squad->commands.empty() ? true : enemy_unit->squad->commands.front()->type != ATTACKING_MOVETO_SQUAD)
					enemy_unit->squad->commands.push_back(new AttackingMoveToSquad(enemy_unit, map_p));
				break;

			case BUILDING:
				App->entitycontroller->HandleSFX(unit->type, 30, true);
				enemy_building = (Building*)enemy;
				enemy_building->current_HP -= MAX((RANDOM_FACTOR * (unit->piercing_atk + ((((int)unit->attack - (int)enemy_unit->defense) <= 0) ? 0 : unit->attack - enemy_building->defense))), 1); //dmg
			}
			timer.Start();
		}

		unit->setDirection((enemy->position - unit->position).Normalized() * MAX_NEXT_STEP_MODULE);
		unit->next_step.SetToZero();
		return true;
	}
	else
	{
		if (path.empty())
		{
			enemy_positions->remove(current_target);
			current_target.SetToZero();
			return true;
		}

		if (map_p == path.front())
			path.pop_front();
		else
			unit->next_step += (path.front() - map_p).Normalized() * STEERING_FACTOR;

	}

	type = ATTACKING_MOVETO;
	return true;
}

bool Attack::OnStop()
{
	type = ATTACKING_MOVETO;
	unit->next_step = { 0,0 };
	return true;
}



//		SQUADS: =============================
//// MOVETOSQUAD

MoveToSquad::MoveToSquad(Unit* commander, iPoint map_dest) : Command(commander, MOVETOSQUAD), dest(map_dest)
{
	if (!unit->squad) { Stop(); return; }
	else squad = unit->squad;

	squad->commander = squad->getClosestUnitTo(dest);
	if (!squad->commander) { Stop(); return; }
};

bool MoveToSquad::OnInit()
{
	if (!flow_field)
	{
		flow_field = App->pathfinding->RequestFlowField(dest);
		state = TO_INIT;
	}
	else
	{
		if(flow_field->stage == FAILED) { Stop(); return true; }
		else 
		{
			for (int i = 0; i < squad->units.size(); i++)
			{
				MoveTo* new_move_order = new MoveTo(squad->units[i], dest, flow_field);
				squad->units[i]->commands.push_back(new_move_order);
			}
			return true;
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
	flow_field->finished = true;
	return true;
}


bool AttackingMoveToSquad::OnUpdate(float dt)
{
	if (!enemies_in_sight)
	{
		if (squad->getEnemiesInSight(enemy_positions) && (just_attacked == false || timer.ReadSec() > 1))
		{
			for (int j = 0; j < squad->units.size(); j++)
			{
				if (squad->units[j]->commands.empty() ? true : (squad->units[j]->commands.front()->type != ATTACK || squad->units[j]->commands.front()->type != ATTACKING_MOVETO))
					squad->units[j]->commands.push_front(new Attack(squad->units[j], &enemy_positions));
			}
			enemies_in_sight = true;
			just_attacked = true;
		}
	}
	else if (enemy_positions.empty())
	{
		if (enemies_in_sight) { enemies_in_sight = false; timer.Start(); }

		if (!hold)
		{
			bool all_idle = true;

			for (int i = 0; i < squad->units.size(); i++)
				if (!squad->units[i]->commands.empty()) all_idle = false;

			if (all_idle) Stop();
		}
	}

	return true;
}

bool AttackingMoveToSquad::OnStop()
{
	if (flow_field)
	{
		flow_field->finished = true;
	}
	return true;
}


bool PatrolSquad::OnUpdate(float dt)
{
	iPoint current_point = patrol_points.front();
	patrol_points.pop_front();

	if (!patrol_points.empty())
	{
		if (App->pathfinding->IsWalkable(current_point))
		{
			iPoint map_p = App->map->WorldToMap(squad->commander->position.x, squad->commander->position.y);

			squad->commands.push_front(new AttackingMoveToSquad(unit, current_point));
			patrol_points.push_back(current_point);
			return true;
		}
	}

	Stop();
	return true;
}


