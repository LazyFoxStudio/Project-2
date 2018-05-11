#include "Command.h"
#include "Unit.h"
#include "Squad.h"
#include "p2Log.h"
#include "j1Pathfinding.h"
#include "j1EntityController.h"
#include "j1ParticleController.h"
#include "j1Map.h"
#include "j1Window.h"
#include "j1Audio.h"
#include "j1Render.h"
#include "j1Scene.h"
// BASE CLASSES: =========================

#define RANDOM_FACTOR (1.0f - (((float)(rand() % 6)) / 10.0f))  // must be the same in Building.cpp
#define STEERING_FACTOR 10.0f

#define PROXIMITY_FACTOR_TILES 1  // the higher the sooner units will reach destination (in tiles)  // 1 ~ 5//
#define PROXIMITY_FACTOR_PIXELS 5

#define MULTITARGET_NUMBER 3
#define TURRET_ROF 1.0f

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
		
		if (useSquadPosition(desired_place))
		{
			unit->mov_target = desired_place;
			if (unit->squad->squad_movement.IsZero() && unit->mov_target.DistanceTo(unit->position) < PROXIMITY_FACTOR_PIXELS)
				Stop();
		}
		else
		{
			if (unit->squad)
				unit->squad->everyone_in_position = false;

			if (map_p.DistanceTo(dest) <= PROXIMITY_FACTOR_TILES)
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

	if(flow_field) flow_field->used_by--;
	return true;
}

bool MoveToFlying::OnUpdate(float dt)
{
	fPoint desired_place = getDesiredPlace();
	map_p = App->map->WorldToMap(unit->position.x, unit->position.y);

	if (!desired_place.IsZero())
	{
		unit->mov_target = desired_place;
		if (unit->squad->squad_movement.IsZero() && map_p.DistanceTo(dest) < PROXIMITY_FACTOR_TILES)
			Stop();
	}

	return true;
}

bool MoveToFlying::OnStop()
{
	unit->mov_module = 0;
	unit->mov_target = unit->position;
	if (unit->squad)
		unit->mov_direction = unit->squad->squad_direction;

	return true;
}


// ATTACK

bool Attack::OnInit()
{
	if (enemy_atk_slots->empty() || !unit->squad || !squad_target) Stop();
	return true;
}

bool Attack::OnUpdate(float dt)
{
	map_p = App->map->WorldToMap(unit->position.x, unit->position.y);

	bool enemy_moved = true;
	for(int i = 0; i < enemy_atk_slots->size(); i++)
		if (current_target == enemy_atk_slots->at(i)){ enemy_moved = false; break; }

	if (current_target.IsZero() || enemy_moved)
		if (!searchTarget()) 
			{ Stop(); return true;}
	

	if (Entity* enemy = App->entitycontroller->getNearestEnemy(unit, *squad_target))
	{
		SDL_Rect r = { unit->position.x - unit->range, unit->position.y - unit->range, unit->range * 2, unit->range * 2 };

		if (SDL_HasIntersection(&r, &enemy->collider) && (!unit->IsMelee() || unit->position.DistanceTo({ (float)current_target.x, (float)current_target.y }) < 5))
		{
			if (type == ATTACKING_MOVETO)
				type = ATTACK; 
			else if (unit->current_anim->justFinished())
			{
				if(App->render->CullingCam(unit->position))
					App->entitycontroller->HandleAttackSFX(unit->type, 30);

				App->entitycontroller->HandleParticles(unit->type, unit->position, enemy->position);

				if (unit->HasAoEDamage())
					AoE_Damage(enemy);
				else
				{
					enemy->current_HP -= dealDamage(unit, enemy); //dmg

					if (enemy->current_HP < 0) { enemy->Destroy(); current_target.SetToZero(); }
					else					   callRetaliation(enemy, unit->squad->UID);
				}
			}

			unit->lookAt(enemy->position - unit->position);
			unit->mov_module = 0;
			return true;
		}
		else
			moveToTarget();
	}
	else Stop();


	type = ATTACKING_MOVETO;
	return true;
}

bool Attack::OnStop()
{
	type = ATTACKING_MOVETO;
	unit->mov_module = 0;

	return true;
}


bool MultiTargetAttack::OnUpdate(float dt)
{
	map_p = App->map->WorldToMap(unit->position.x, unit->position.y);

	bool enemy_moved = true;
	for (int i = 0; i < enemy_atk_slots->size(); i++)
		if (current_target == enemy_atk_slots->at(i)) { enemy_moved = false; break; }

	if (current_target.IsZero() || enemy_moved)
		if (!searchTarget())
			Stop();

	std::vector<Entity*> enemies;
	if (App->entitycontroller->getNearestEnemies(unit, *squad_target, MULTITARGET_NUMBER, enemies))
	{
		SDL_Rect r = { unit->position.x - unit->range, unit->position.y - unit->range, unit->range * 2, unit->range * 2 };
		bool has_attacked = false;

		for (int i = 0; i < enemies.size(); i++)
		{
			if (SDL_HasIntersection(&r, &enemies[i]->collider))
			{
				if (type == ATTACKING_MOVETO)
					{type = ATTACK; return true;}
				else if (unit->timer.ReadSec() > TURRET_ROF)
				{
					if (App->render->CullingCam(unit->position))
						App->entitycontroller->HandleAttackSFX(unit->type, 30);

					if (i == 0)
					{
						AoE_Damage(enemies[i]);
						App->entitycontroller->HandleParticles(unit->type, unit->position,  enemies[i]->position);
						unit->lookAt(enemies[i]->position - unit->position);
						unit->mov_module = 0;
					}
					else
					{
						enemies[i]->current_HP -= dealDamage(unit, enemies[i]); //dmg
						App->entitycontroller->HandleParticles(ARCHER, unit->position, enemies[i]->position);

						if (enemies[i]->current_HP < 0) enemies[i]->Destroy();
						else					   callRetaliation(enemies[i], unit->squad->UID);
					}
					has_attacked = true;
				}
			}
			else
			{
				moveToTarget();
				break;
			}
		}
		if (has_attacked)
		{
			unit->timer.Start();
			return true;
		}
	}
	else Stop();


	type = ATTACKING_MOVETO;
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
				iPoint world_parent = App->map->MapToWorld(parent->position.x, parent->position.y);
				world_parent += {App->map->data.tile_width / 2, App->map->data.tile_height / 2};

				fPoint movement = fPoint( (float)world_parent.x, (float)world_parent.y ) - commander->position;
				if (parent->position == dest)
				{
					movement = movement.Normalized() * dt * squad->max_speed * SPEED_CONSTANT;
					squad->squad_movement = ((squad->squad_movement * STEERING_FACTOR) + movement);

					if (squad->squad_movement.GetModule() > movement.GetModule())
						squad->squad_movement = squad->squad_movement.Normalized() * movement.GetModule();
				}
				else
					squad->squad_movement = movement;
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

bool MoveToSquadFlying::OnInit()
{
	if (!unit->squad) { Stop(); return true; }
	else {
		squad = unit->squad;

		std::vector<Unit*> squad_units;
		squad->getUnits(squad_units);
		squad->squad_movement.SetToZero();

		if (!squad_units.empty())
		{
			for (int i = 0; i < squad_units.size(); i++)
			{
				MoveToFlying* new_move_order = new MoveToFlying(squad_units[i], dest);
				squad_units[i]->commands.push_back(new_move_order);
			}
		}
	}

	return true;
}

bool MoveToSquadFlying::OnUpdate(float dt)
{
	if (allIdle()) Stop();
	else {
		if (Unit* commander = squad->getCommander())
		{
			iPoint map_p = App->map->WorldToMap(commander->position.x, commander->position.y);
			iPoint world_parent = App->map->MapToWorld(map_p.x, map_p.y);
			world_parent += {App->map->data.tile_width / 2, App->map->data.tile_height / 2};

			fPoint movement = fPoint((float)world_parent.x, (float)world_parent.y) - commander->position;
			if (map_p != dest)
			{
				movement = movement.Normalized() * dt * squad->max_speed * SPEED_CONSTANT;
				squad->squad_movement = ((squad->squad_movement * STEERING_FACTOR) + movement);

				if (squad->squad_movement.GetModule() > movement.GetModule())
					squad->squad_movement = squad->squad_movement.Normalized() * movement.GetModule();
			}
			else
				squad->squad_movement = movement;
		}
	}

	return true;
}

bool MoveToSquadFlying::OnStop()
{
	if (squad) squad->squad_movement.SetToZero();

	return true;
}


bool AttackingMoveToSquad::OnUpdate(float dt)
{
	squad->findAttackSlots(enemy_atk_slots, target_squad_id);

	if (!enemy_atk_slots.empty())
	{
		enemies_found = true;

		std::vector<Unit*> units;
		squad->getUnits(units);

		for (int i = 0; i < units.size(); i++)
		{
			if (units[i]->commands.empty() ? true : (units[i]->getCurrentCommand() != ATTACK && units[i]->getCurrentCommand() != ATTACKING_MOVETO))
			{
				if(units[i]->type == JUGGERNAUT)	units[i]->commands.push_front(new MultiTargetAttack(units[i], &enemy_atk_slots, &target_squad_id));
				else								units[i]->commands.push_front(new Attack(units[i], &enemy_atk_slots, &target_squad_id));
			}
				
		}

	}
	else
	{
		if (!enemies_found || (unit->IsEnemy() || hold))
		{
			if (Unit* commander = squad->getCommander())
			{
				iPoint map_p = App->map->WorldToMap(commander->position.x, commander->position.y);

				if (FieldNode* parent = flow_field->getNodeAt(map_p)->parent)
				{
					iPoint world_parent = App->map->MapToWorld(parent->position.x, parent->position.y);
					world_parent += {App->map->data.tile_width / 2, App->map->data.tile_height / 2};

					fPoint movement = fPoint((float)world_parent.x, (float)world_parent.y) - commander->position;
					if (parent->position == dest)
					{
						movement = movement.Normalized() * dt * squad->max_speed * SPEED_CONSTANT;
						squad->squad_movement = ((squad->squad_movement * STEERING_FACTOR) + movement);

						if (squad->squad_movement.GetModule() > movement.GetModule())
							squad->squad_movement = squad->squad_movement.Normalized() * movement.GetModule();
					}
					else
						squad->squad_movement = movement;
				}
			}
		}
		else if (enemies_found && target_squad_id != -1)
			target_squad_id = -1;
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



bool AttackingMoveToSquadFlying::OnUpdate(float dt)
{
	squad->findAttackSlots(enemy_atk_slots, target_squad_id);

	if (!enemy_atk_slots.empty())
	{
		enemies_found = true;

		std::vector<Unit*> units;
		squad->getUnits(units);

		for (int i = 0; i < units.size(); i++)
		{
			if (units[i]->commands.empty() ? true : (units[i]->getCurrentCommand() != ATTACK && units[i]->getCurrentCommand() != ATTACKING_MOVETO))
				units[i]->commands.push_front(new Attack(units[i], &enemy_atk_slots, &target_squad_id));
		}

	}
	else
	{
		if (!enemies_found || (unit->IsEnemy() || hold))
		{
			if (Unit* commander = squad->getCommander())
			{
				iPoint map_p = App->map->WorldToMap(commander->position.x, commander->position.y);
				iPoint world_parent = App->map->MapToWorld(map_p.x, map_p.y);
				world_parent += {App->map->data.tile_width / 2, App->map->data.tile_height / 2};

				fPoint movement = fPoint((float)world_parent.x, (float)world_parent.y) - commander->position;
				if (map_p != dest)
				{
					movement = movement.Normalized() * dt * squad->max_speed * SPEED_CONSTANT;
					squad->squad_movement = ((squad->squad_movement * STEERING_FACTOR) + movement);

					if (squad->squad_movement.GetModule() > movement.GetModule())
						squad->squad_movement = squad->squad_movement.Normalized() * movement.GetModule();
				}
				else
					squad->squad_movement = movement;
			}
		}
		else if (enemies_found && target_squad_id != -1)
			target_squad_id = -1;
		else
		{
			std::vector<Unit*> units;
			squad->getUnits(units);

			for (int i = 0; i < units.size(); i++)
				units[i]->Halt();

			Stop();
		}
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

	fPoint place = unit->squad->commander_pos + unit->squad->getOffset(unit->UID);
	if (!place.IsZero())
	{
		iPoint map_place = App->map->WorldToMap(place.x, place.y);
		if (App->pathfinding->IsWalkable(map_place))
			return place;
	}

	return { 0.0f, 0.0f };
}

bool MoveTo::useSquadPosition(fPoint desired_place)
{
	if (desired_place.IsZero())
		return false;
	else
	{
		if (desired_place.DistanceTo(unit->position) > SQUAD_UNATTACH_DISTANCE)
		{
			iPoint world_dest = App->map->MapToWorld(dest.x, dest.y);
			if ((unit->position.DistanceTo({ (float)world_dest.x, (float)world_dest.y }) > desired_place.DistanceTo({ (float)world_dest.x, (float)world_dest.y })))
				return false;
		}
	}


	return true;
}

fPoint MoveToFlying::getDesiredPlace()
{
	return unit->squad->commander_pos + unit->squad->getOffset(unit->UID);
}

bool Attack::searchTarget()
{
	if (enemy_atk_slots->empty()) { Stop(); return false; }

	std::vector<iPoint> slots_in_use;
	std::vector<Unit*> units;
	unit->squad->getUnits(units);

	for (int i = 0; i < units.size(); i++)
	{
		if (units[i]->getCurrentCommand() == ATTACK || units[i]->getCurrentCommand() == ATTACKING_MOVETO)
			slots_in_use.push_back(((Attack*)units[i]->commands.front())->current_target);
	}

	current_target.SetToZero();
	
	for (int i = 0; i < enemy_atk_slots->size(); i++)
	{
		bool used = false;
		for (int j = 0; j < slots_in_use.size(); j++)
		{
			if(slots_in_use[j] == enemy_atk_slots->at(i))
				{ used = true; break;}
		}
		if (used) continue;

		if (enemy_atk_slots->at(i).DistanceTo({ (int)unit->position.x, (int)unit->position.y }) < current_target.DistanceTo({ (int)unit->position.x, (int)unit->position.y }))
			current_target = enemy_atk_slots->at(i);

	}

	if (current_target.IsZero()) current_target = enemy_atk_slots->at(rand() % enemy_atk_slots->size());

	if (!unit->IsFlying())
	{
		iPoint targetMap_p = App->map->WorldToMap(current_target.x, current_target.y);

		if (!App->pathfinding->IsWalkable(targetMap_p) && unit->IsRanged())
		{
			targetMap_p = App->pathfinding->FirstWalkableAdjacent(targetMap_p);
			iPoint world_p = App->map->MapToWorld(targetMap_p.x, targetMap_p.y);

			if(world_p.DistanceTo(current_target) > (unit->range - App->map->data.tile_width))
				{ Stop(); return true; }
		}

		if (!App->pathfinding->CreatePath(map_p, targetMap_p) >= 0) path = *App->pathfinding->GetLastPath();
		else { Stop(); return true; }
	}

	return true;
}

void Attack::moveToTarget()
{
	if (!unit->IsFlying())
	{
		if (!path.empty())
		{
			iPoint world_p = App->map->MapToWorld(path.front().x, path.front().y);
			world_p = { world_p.x + App->map->data.tile_width / 2, world_p.y + App->map->data.tile_height / 2 };

			if (unit->position.DistanceTo({ (float)world_p.x, (float)world_p.y }) < 5)
				path.pop_front();
			else
				unit->mov_target = { (float)world_p.x , (float)world_p.y};

		}
	}
	else
		unit->mov_target = { (float)current_target.x, (float)current_target.y };
		
}

void Attack::callRetaliation(Entity* enemy, uint squad_UID)
{
	for (std::list<Squad*>::iterator it = App->entitycontroller->squads.begin(); it != App->entitycontroller->squads.end(); it++)
	{
		if (Unit* enemy_commander = (*it)->getCommander())
		{
			if (enemy_commander->IsEnemy() == enemy->IsEnemy() && enemy_commander->position.DistanceTo(enemy->position) < enemy_commander->line_of_sight)
			{
				Command_Type enemy_action = (*it)->getCurrentCommand();
				if (enemy_action == NOTHING)
					(*it)->commands.push_back(new AttackingMoveToSquad(enemy_commander, map_p, false, squad_UID));
			}
		}
	}

}

int Attack::dealDamage(Entity * attacker, Entity * defender)
{
	int ret = 0;
	
		if (favorableMatchup(attacker, defender))
		{
			ret = MATCHUP_MODIFIER * MAX((RANDOM_FACTOR * (attacker->piercing_atk + ((((int)attacker->attack - (int)defender->defense) <= 0) ? 0 : attacker->attack - defender->defense))), 1);
		}

		else
		{
			ret = MAX((RANDOM_FACTOR * (attacker->piercing_atk + ((((int)attacker->attack - (int)defender->defense) <= 0) ? 0 : attacker->attack - defender->defense))), 1);
		}
	return ret;
}

void Attack::AoE_Damage(Entity* enemy)
{
	int distance = App->map->data.tile_width * 3;
	SDL_Rect damage_area = { enemy->position.x - distance / 2, enemy->position.y - distance / 2, distance, distance };
	std::vector<Entity*> colliding_entities;

	App->entitycontroller->CheckCollidingWith(damage_area, colliding_entities);

	for (int i = 0; i < colliding_entities.size(); i++)
	{
		if (colliding_entities[i]->IsEnemy() != unit->IsEnemy())
		{
			colliding_entities[i]->current_HP -= dealDamage(unit, colliding_entities[i]); //dmg

			if (colliding_entities[i]->current_HP < 0)	colliding_entities[i]->Destroy();
			else										callRetaliation(colliding_entities[i], unit->squad->UID);
		}
	}
}

bool Attack::favorableMatchup(Entity * attacker, Entity * defender)
{
	bool ret = false;
	if (attacker->range == 40 && defender->range < 40 && !defender->flying)
	{
		ret = true;
	}
	else if (attacker->range < 40 && !attacker->flying && defender->flying)
	{
		ret = true;
	}
	else if (attacker->flying && !defender->flying && defender->range == 40)
	{
		ret = true;
	}
	return ret;
}



bool MoveToSquad::allIdle()
{
	std::vector<Unit*> units;
	squad->getUnits(units);

	for (int i = 0; i < units.size(); i++)
		if (!units[i]->commands.empty()) return false;

	return true;
}

bool MoveToSquadFlying::allIdle()
{
	std::vector<Unit*> units;
	squad->getUnits(units);

	for (int i = 0; i < units.size(); i++)
		if (!units[i]->commands.empty()) return false;

	return true;
}
