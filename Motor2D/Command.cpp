#include "Command.h"
#include "Unit.h"
#include "Squad.h"
#include "p2Log.h"
#include "j1Pathfinding.h"
#include "j1EntityController.h"
#include "j1ParticleController.h"
#include "j1Map.h"
#include "j1Audio.h"
#include "j1Scene.h"
// BASE CLASSES: =========================

#define RANDOM_FACTOR (1.0f - (((float)(rand() % 6)) / 10.0f))
#define STEERING_FACTOR 10.0f

#define PROXIMITY_FACTOR_TILES 2  // the higher the sooner units will reach destination (in tiles)  // 1 ~ 5//
#define PROXIMITY_FACTOR_PIXELS 5

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
	else flow_field->used_by++;

	return true;
}

bool MoveTo::OnUpdate(float dt)
{
	if (flow_field->stage == FAILED)
		Stop();
	else
	{
		fPoint desired_place = getDesiredPlace();
		map_p = App->map->WorldToMap(unit->position.x, unit->position.y);

		if (!desired_place.IsZero() && desired_place.DistanceTo(unit->position) < SQUAD_UNATTACH_DISTANCE && (unit->squad ? unit->squad->units_id.size() > 1 : false))
		{
			unit->mov_target = desired_place;
			if (unit->squad->squad_movement.IsZero() && unit->mov_target.DistanceTo(unit->position) < PROXIMITY_FACTOR_PIXELS)
				Stop();
		}
		else
		{
			if (map_p.DistanceTo(dest) < PROXIMITY_FACTOR_TILES)
				Stop();
			else if (FieldNode* parent = flow_field->getNodeAt(map_p)->parent)
			{
				iPoint world_p = App->map->MapToWorld(parent->position.x, parent->position.y);
				unit->mov_target = unit->position + ((fPoint{ (float)world_p.x + App->map->data.tile_width / 2, (float)world_p.y + App->map->data.tile_height / 2 } -unit->position).Normalized()) * 10;
			}
			
		}
	}

	return true; 
}


bool MoveTo::OnStop()
{
	unit->mov_module = 0;
	unit->mov_target = unit->position;
	if(unit->squad) 
		unit->mov_direction = unit->squad->squad_direction;

	if(flow_field) flow_field->used_by++;
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
	{
		if (!searchTarget()) Stop();
		return true;
	}
	
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
			//App->particle->AddProjectile(particleType::TOMAHAWK, unit->position, enemy->position, 200);
			enemy->current_HP -= MAX((RANDOM_FACTOR * (unit->piercing_atk + ((((int)unit->attack - (int)enemy->defense) <= 0) ? 0 : unit->attack - enemy->defense))), 1); //dmg

			if (enemy->current_HP < 0) enemy->Destroy();
			else					   callRetaliation(enemy);

			timer.Start();
		}

		unit->lookAt(enemy->position - unit->position);
		unit->mov_module = 0;
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
	unit->mov_module = 0;

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
	else if(flow_field->stage == FAILED) { Stop(); return true; }
	

	return true;
}

bool MoveToSquad::OnUpdate(float dt)
{
	if ((allIdle() && launched) || flow_field->stage == FAILED) 
		Stop();
	else {
		if (Unit* commander = squad->getCommander())
		{
			iPoint map_p = App->map->WorldToMap(commander->position.x, commander->position.y);

			if (FieldNode* parent = flow_field->getNodeAt(map_p)->parent)
			{
				fPoint movement = (parent->position - map_p).Normalized() * dt * squad->max_speed * SPEED_CONSTANT;
				squad->squad_movement = ((squad->squad_movement * STEERING_FACTOR) + movement);

				if (squad->squad_movement.GetModule() > movement.GetModule())
					squad->squad_movement = squad->squad_movement.Normalized() * movement.GetModule();
			}
			else if(launched)
				Stop();
	
		}
	}
	if (!launched && !squad->squad_movement.IsZero()) 
		Launch();

	return true;
}


bool MoveToSquad::OnStop()
{
	if(!unit->IsEnemy() && flow_field) flow_field->used_by--;
	if(squad) squad->squad_movement.SetToZero();

	return true;
}


bool AttackingMoveToSquad::OnUpdate(float dt)
{
	squad->getEnemiesInSight(enemy_ids, target_squad_id);

	if (!enemy_ids.empty())
	{
		if (target_squad_id != -1) target_squad_id = -1;
		enemies_found = true;

		std::vector<Unit*> units;
		squad->getUnits(units);

		for (int i = 0; i < units.size(); i++)
		{
			if (units[i]->commands.empty() ? true : (units[i]->getCurrentCommand() != ATTACK && units[i]->getCurrentCommand() != ATTACKING_MOVETO))
				units[i]->commands.push_front(new Attack(units[i], &enemy_ids));
		}

	}
	else
	{
		if (!enemies_found || unit->IsEnemy())
		{
			if (Unit* commander = squad->getCommander())
			{
				iPoint map_p = App->map->WorldToMap(commander->position.x, commander->position.y);

				if (FieldNode* parent = flow_field->getNodeAt(map_p)->parent)
				{
					fPoint movement = (parent->position - map_p).Normalized() * dt * squad->max_speed * SPEED_CONSTANT;
					squad->squad_movement = ((squad->squad_movement * STEERING_FACTOR) + movement);

					if (squad->squad_movement.GetModule() > movement.GetModule())
						squad->squad_movement = squad->squad_movement.Normalized() * movement.GetModule();
				}
			}
		}
		else
		{
			std::vector<Unit*> units;
			squad->getUnits(units);

			for (int i = 0; i < units.size(); i++)
				units[i]->Halt();

			Stop();
		}
	}

	if (!launched && !squad->squad_movement.IsZero()) 
		Launch();

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

void MoveToSquad::Launch()
{
	std::vector<Unit*> squad_units;
	squad->getUnits(squad_units);
	squad->squad_movement.SetToZero();

	if (!squad_units.empty())
	{
		for (int i = 0; i < squad_units.size(); i++)
		{
			MoveTo* new_move_order = new MoveTo(squad_units[i], dest, flow_field);
			squad_units[i]->commands.push_back(new_move_order);
		}
	}

	launched = true;
}

fPoint MoveTo::getDesiredPlace()
{
	if (unit->squad)
	{
		fPoint place = unit->squad->commander_pos + unit->squad->getOffset(unit->UID);
		if (!place.IsZero())
		{
			iPoint map_place = App->map->WorldToMap(place.x, place.y);
			if (App->pathfinding->IsWalkable(map_place))
				return place;
		}
	}
	return { 0.0f, 0.0f };
}

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
			Entity* new_target = App->entitycontroller->getEntitybyID(enemy_ids->at(i));

			if (new_target->position.DistanceTo(unit->position) < target->position.DistanceTo(unit->position))
				target = new_target;
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
	{
		iPoint world_p = App->map->MapToWorld(path.front().x, path.front().y);
		unit->mov_target = { (float)world_p.x + App->map->data.tile_width / 2, (float)world_p.y + App->map->data.tile_height /2};
	}
		
}

void Attack::callRetaliation(Entity* enemy)
{
	if (enemy->IsUnit())
	{
		Command_Type enemy_action = ((Unit*)enemy)->squad->getCurrentCommand();
		if (enemy_action != ATTACKING_MOVETO_SQUAD)
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
