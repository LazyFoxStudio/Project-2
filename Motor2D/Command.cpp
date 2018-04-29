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
	if (enemy_ids->empty()) Stop();

	return true;
}

bool Attack::OnUpdate(float dt)
{
	map_p = App->map->WorldToMap(unit->position.x, unit->position.y);

	if (current_target == -1)
		return searchTarget();
	
	Entity* enemy = App->entitycontroller->getEntitybyID(current_target);

	if (!enemy)												   { Stop(); return true; }
	else if (enemy->ex_state == DESTROYED || !enemy->isActive) { current_target = -1; return true; }

	SDL_Rect r = { unit->position.x - unit->range, unit->position.y - unit->range, unit->range * 2, unit->range * 2};

	if(SDL_HasIntersection(&r, &enemy->collider) && enemy->isActive)
	{
		if (type == ATTACKING_MOVETO) 
			{ type = ATTACK; timer.Start(); }
		else if (unit->current_anim->justFinished())
		{ 
			App->entitycontroller->HandleSFX(unit->type, 30);
			enemy->current_HP -= MAX((RANDOM_FACTOR * (unit->piercing_atk + ((((int)unit->attack - (int)enemy->defense) <= 0) ? 0 : unit->attack - enemy->defense))), 1); //dmg

			if (enemy->current_HP < 0) enemy->Destroy();
			else					   callRetaliation(enemy);

			timer.Start();
		}

		unit->lookAt((enemy->position - unit->position).Normalized() * MAX_NEXT_STEP_MODULE);
		unit->next_step.SetToZero();
		return true;
	}
	else
		moveToTarget();

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
			std::vector<Unit*> squad_units;
			squad->getUnits(squad_units);

			if (!squad_units.empty())
			{
				for (int i = 0; i < squad_units.size(); i++)
				{
					MoveTo* new_move_order = new MoveTo(squad_units[i], dest, flow_field);
					squad_units[i]->commands.push_back(new_move_order);
				}
			}
			return true;
		}
	}
	
	return true;
}

bool MoveToSquad::OnUpdate(float dt)
{
	if (allIdle()) Stop();

	return true;
}


bool MoveToSquad::OnStop()
{
	flow_field->finished = true;
	return true;
}


bool AttackingMoveToSquad::OnUpdate(float dt)
{
	squad->getEnemiesInSight(enemy_ids, target_squad_id);

	if (!enemy_ids.empty())
	{
		std::vector<Unit*> units;
		squad->getUnits(units);

		for (int i = 0; i < units.size(); i++)
		{
			if (units[i]->commands.empty() ? true : (units[i]->commands.front()->type != ATTACK && units[i]->commands.front()->type != ATTACKING_MOVETO))
				units[i]->commands.push_front(new Attack(units[i], &enemy_ids));
		}

	}
	else if(!hold && allIdle()) Stop();

	return true;
}

bool AttackingMoveToSquad::OnStop()
{
	if (flow_field) flow_field->finished = true;

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
			if (Unit* commander = squad->getCommander())
			{
				iPoint map_p = App->map->WorldToMap(commander->position.x, commander->position.y);

				squad->commands.push_front(new AttackingMoveToSquad(unit, current_point));
				patrol_points.push_back(current_point);
				return true;
			}
		}
	}

	Stop();
	return true;
}



// UTILITY METHODS


bool AttackingMoveToSquad::checkSquadTarget()
{
	for (int i = 0; i < enemy_ids.size(); i++)
	{
		Entity* enemy = App->entitycontroller->getEntitybyID(enemy_ids[i]);
		if (enemy->IsUnit() ? ((Unit*)enemy)->squad->UID == target_squad_id : false)
			return true;
	}
	return true;
}

bool Attack::searchTarget()
{
	if (enemy_ids->empty()) { Stop(); return false; }
	
	Entity* target = App->entitycontroller->getEntitybyID(enemy_ids->front());
	if (target != nullptr)
	{
		for (int i = 1; i < enemy_ids->size(); i++)
		{
			if (target->position.DistanceTo(unit->position) < App->entitycontroller->getEntitybyID(enemy_ids->at(i))->position.DistanceTo(unit->position))
				target = App->entitycontroller->getEntitybyID(enemy_ids->at(i));
		}

		iPoint targetMap_p = App->map->WorldToMap(target->position.x, target->position.y);
		if (!App->pathfinding->CreatePath(map_p, targetMap_p) >= 0) path = *App->pathfinding->GetLastPath();
		else { Stop(); return false; }
	}
	else 
		{ Stop(); return false; }

	current_target = target->UID;
	return true;
}

void Attack::moveToTarget()
{
	if (path.empty())
	{
		current_target = -1;
		return;
	}

	if (map_p == path.front())
		path.pop_front();
	else
		unit->next_step += (path.front() - map_p).Normalized() * STEERING_FACTOR;
}

void Attack::callRetaliation(Entity* enemy)
{
	if (enemy->IsUnit())
	{
		Command_Type enemy_action = ((Unit*)enemy)->squad->getCurrentCommand();
		if (enemy_action != ATTACK && enemy_action != ATTACKING_MOVETO)
			((Unit*)enemy)->squad->commands.push_back(new AttackingMoveToSquad((Unit*)enemy, map_p));
	}
}



bool MoveToSquad::allIdle()
{
	std::vector<Unit*> units;
	squad->getUnits(units);

	for (int i = 0; i < units.size(); i++)
		if (!units[i]->commands.empty()) return false;

	return true;
}
