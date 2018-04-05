#include "Command.h"
#include "Unit.h"
#include "Squad.h"
#include "p2Log.h"
#include "j1Pathfinding.h"
#include "j1EntityController.h"
#include "j1Map.h"

#define SPEED_CONSTANT 100   // applied to all units
#define STEERING_FACTOR 7.0f    // the higher the stiffer
#define SEPARATION_STRENGTH 5.0f   // the higher the stronger
#define PROXIMITY_FACTOR 3  // the higher the sooner units will stop moving

#define MAX_NEXT_STEP_MODULE 25
#define COLLIDER_MARGIN 25

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
	if (!flow_field)
	{
		map_p = App->map->WorldToMap(unit->position.x, unit->position.y);
		flow_field = App->pathfinding->CreateFlowField(map_p, dest);
		unique_field = true;

		if (!flow_field) Stop();
	}
	

	return true;
}

bool MoveTo::OnUpdate(float dt)
{
	fPoint last_pos = unit->position;

	if (unit->squad)	unit->position += (unit->next_step.Normalized() * unit->squad->max_speed * dt * SPEED_CONSTANT);
	else				unit->position += (unit->next_step.Normalized() * unit->speed * dt * SPEED_CONSTANT);

	map_p = App->map->WorldToMap(unit->position.x, unit->position.y);

	if(!App->pathfinding->IsWalkable(map_p)) unit->position = last_pos;
	else
	{
		unit->collider.x = unit->position.x - (unit->collider.w / 2);
		unit->collider.y = unit->position.y - (unit->collider.h / 2);
	}

	if (map_p.DistanceTo(dest) < PROXIMITY_FACTOR)
	{
		if (!checkCollisionsAlongPath(map_p)) Stop();
	}
	else
	{
		if (!flow_field->getNodeAt(map_p)->parent) Stop();
		else
		{
			fPoint direction	= (flow_field->getNodeAt(map_p)->parent->position - map_p).Normalized();
			unit->next_step		= unit->next_step + ((direction + calculateSeparationVector()) * STEERING_FACTOR);

			if (unit->next_step.GetModule() > MAX_NEXT_STEP_MODULE)
				unit->next_step = unit->next_step.Normalized() * MAX_NEXT_STEP_MODULE;
		}
	}

	return true; 
}


bool MoveTo::OnStop()
{
	if (flow_field && unique_field) delete flow_field;
	unit->next_step = { 0,0 };

	return true;
}

fPoint MoveTo::calculateSeparationVector()
{
	SDL_Rect r = { unit->collider.x - COLLIDER_MARGIN, unit->collider.y - COLLIDER_MARGIN , unit->collider.w + COLLIDER_MARGIN , unit->collider.h + COLLIDER_MARGIN };
	std::vector<Entity*> collisions = App->entitycontroller->CheckCollidingWith(r, unit);

	fPoint separation_v = { 0,0 };
	for (int i = 0; i < collisions.size(); i++)
	{
		if (collisions[i]->entity_type == UNIT)
		{
			fPoint current_separation = (unit->position - collisions[i]->position);
			separation_v += current_separation.Normalized() * (1 / current_separation.GetModule());
		}
	}
	
	return separation_v * SEPARATION_STRENGTH;
}

bool MoveTo::checkCollisionsAlongPath(iPoint origin)
{
	if (flow_field)
	{
		FieldNode* fn = flow_field->getNodeAt(origin);
		SDL_Rect r = { 0,0, App->map->data.tile_width, App->map->data.tile_height};

		if (fn)
		{
			while (fn->parent)
			{
				fn = fn->parent;
				iPoint world_p = App->map->MapToWorld(fn->position.x, fn->position.y);
				r.x = world_p.x; r.y = world_p.y;

				std::vector<Entity*> collisions = App->entitycontroller->CheckCollidingWith(r, unit);

				if (collisions.empty()) return true;
			}
		}
	}

	return false;
}


// ATTACKING MOVE TO

bool AttackingMoveTo::OnUpdate(float dt) 
{
	Unit* enemy = unit->SearchNearestEnemy();
	if (enemy)
	{
		iPoint enemy_position = App->map->WorldToMap(enemy->position.x, enemy->position.y);
		iPoint pos = App->map->WorldToMap(unit->position.x, unit->position.y);

		if (pos.DistanceTo(enemy_position) < unit->line_of_sight)
		{
			Attack* new_attack_command = new Attack(unit);
			new_attack_command->enemy_position = enemy_position;
			new_attack_command->state = UPDATE;
			unit->commands.push_front(new_attack_command);

			state = TO_INIT;
			return true;
		}
	}

	unit->position += unit->next_step;
	unit->collider.x = unit->position.x; unit->collider.y = unit->position.y;

	iPoint unit_pos = App->map->WorldToMap(unit->position.x, unit->position.y);

	if (unit_pos == path.front())
	{
		path.pop_front();
		if (path.empty()) { Stop(); return true; }
	}

	iPoint direction = path.front() - unit_pos;
	direction.Normalize();

	fPoint velocity = (direction * unit->speed * dt * SPEED_CONSTANT);
	unit->next_step = velocity;

	return true;
}


// ATTACK

bool Attack::OnInit()
{
	Unit* enemy = unit->SearchNearestEnemy();
	if (enemy)
	{
		enemy_position = App->map->WorldToMap(enemy->position.x, enemy->position.y);
		iPoint pos     = App->map->WorldToMap(unit->position.x, unit->position.y);

		if (pos.DistanceTo(enemy_position) > unit->line_of_sight)	Stop();
	}
	else Stop();

	return true;
}

bool Attack::OnUpdate(float dt)
{
	iPoint pos = App->map->WorldToMap(unit->position.x, unit->position.y);

	if (pos.DistanceTo(enemy_position) <= unit->range)
	{
		// if(unit->current_animation = unit->animations[ATTACK_ANIMATION])
		//TODO
	}
	return true;
}

bool Attack::OnStop()
{
	// TODO
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
	Unit* enemy = unit->SearchNearestEnemy();
	if (enemy)
	{
		iPoint enemy_position = App->map->WorldToMap(enemy->position.x, enemy->position.y);
		iPoint pos = App->map->WorldToMap(unit->position.x, unit->position.y);

		if (pos.DistanceTo(enemy_position) < unit->line_of_sight)
		{
			Attack* new_attack_command = new Attack(unit);
			new_attack_command->enemy_position = enemy_position;
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

// RESHAPE

bool ReshapeSquad::OnInit()
{
	if (!unit->squad) { Stop(); return true; }
	else squad = unit->squad;

	iPoint commander_pos = App->map->WorldToMap(squad->commander->position.x, squad->commander->position.y);
	std::list<iPoint> adjacents;

	if (App->pathfinding->GatherWalkableAdjacents(commander_pos, squad->units.size() - 1, adjacents))
	{
		for (int i = 1; i < squad->units.size(); i++)
		{
			iPoint dest = adjacents.front();
			iPoint target = commander_pos + squad->unit_offset[i];

			for (std::list<iPoint>::iterator it = adjacents.begin(); it != adjacents.end(); it++)
				if ((*it).DistanceManhattan(target) < dest.DistanceManhattan(target)) dest = (*it);

			squad->units[i]->commands.push_front(new MoveTo(squad->units[i], dest));
			adjacents.remove(dest);
		}
	}
	else Stop();

	return true;
}

bool ReshapeSquad::OnUpdate(float dt)
{
	bool all_idle = true;

	for (int i = 0; i < squad->units.size(); i++)
		if (!squad->units[i]->commands.empty()) all_idle = false;

	if (all_idle) 
		Stop();

	return true;
}
