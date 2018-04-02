#include "Command.h"
#include "Unit.h"
#include "Squad.h"
#include "p2Log.h"
#include "j1Pathfinding.h"
#include "j1EntityController.h"
#include "j1Map.h"

#define SPEED_CONSTANT 100


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
	next_step = { 0.0f,0.0f };
	if (path.empty()) 
	{
		iPoint pos = App->map->WorldToMap(unit->position.x, unit->position.y);

		if (App->pathfinding->CreatePath(pos, dest) > 0)	path = *App->pathfinding->GetLastPath();
		else												Stop();
	}
	else Repath();    // if we have been already initalized and possess a path, repath

	return true;
}

bool MoveTo::OnUpdate(float dt)
{
	if (path.empty()) { Stop(); return true; }
	else {
		if (!waiting_for)
		{
			unit->position += next_step;
			unit->collider.x = unit->position.x; unit->collider.y = unit->position.y;

			iPoint unit_pos = App->map->WorldToMap(unit->position.x, unit->position.y);

			iPoint direction = path.front() - unit_pos;
			direction.Normalize();

			if (unit->squad)	next_step = (direction * unit->squad->max_speed * dt * SPEED_CONSTANT);
			else				next_step = (direction * unit->speed * dt * SPEED_CONSTANT);

			if (unit_pos == path.front())
				path.pop_front();
		}
	}

	if (!CheckCollisions()) Stop();

	return true;
}

bool MoveTo::OnStop()
{
	if (!path.empty())	path.clear();
	return true;
}

bool MoveTo::CheckCollisions()
{
	waiting_for = nullptr;
	std::vector<Entity*> collisions = App->entitycontroller->CheckCollidingWith(unit);

	if (!collisions.empty())
	{
		if (collisions.size() == 1)
		{
			if (collisions.front()->entity_type != UNIT)
			{
				if (unit->squad) { unit->squad->Halt(); unit->squad->commands.push_back(new MoveToSquad(unit->squad->commander, dest)); }
				else { unit->Halt(); unit->commands.push_back(new MoveTo(unit, dest)); }
			}
			else {
				Unit* colliding_unit = (Unit*)collisions.front();
				if (colliding_unit->commands.empty())  return colliding_unit->Pushed(next_step);
				else
				{
					MoveTo * colliding_moveTo = nullptr;
					switch (colliding_unit->commands.front()->type)
					{
					case MOVETO:
						colliding_moveTo = (MoveTo*)colliding_unit->commands.front();
						if (!colliding_moveTo->waiting_for)
							if(colliding_moveTo->waiting_for != unit) waiting_for = colliding_unit;
						else {
							if (colliding_moveTo->next_step.Normalized() == next_step.Normalized().Negate())
							{
								if (colliding_unit->Pushed(next_step)) { colliding_unit->commands[1]->state = TO_INIT; return true; }
								else {
									if (unit->Pushed(colliding_moveTo->next_step)) { state = TO_INIT; return true; }
									else										   return false;
								}
							}
							else waiting_for = colliding_unit;
						}
						break;
					case ATTACK:
						state = TO_INIT;
						return unit->Pushed(next_step.Negate());
						break;
					default:
						return colliding_unit->Pushed(next_step);
						break;
					}
				}
			}
		}
		else {   // collisions.size() > 1
			//TODO;
		}
	}

	return true;
}

void MoveTo::Repath()
{
	iPoint pos = App->map->WorldToMap(unit->position.x, unit->position.y);

	std::list<iPoint> new_path;
	if (App->pathfinding->CreatePath(pos, path.front()) > 0)
	{
		new_path = *App->pathfinding->GetLastPath();

		for (std::list<iPoint>::reverse_iterator it = new_path.rbegin(); it != new_path.rend(); it++)
			path.push_front(*it);
	}
	else if (App->pathfinding->CreatePath(pos, dest) > 0)
		path = *App->pathfinding->GetLastPath();
	else
		Stop();
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

	unit->position += next_step;
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
	next_step = velocity;

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
// MOVETOSQUAD

bool MoveToSquad::OnInit()
{
	if (!unit->squad) { Stop(); return true; }
	else squad = unit->squad;

	if (!reshaping_done)
	{
		squad->commands.push_front(new ReshapeSquad(squad->commander));
		state = TO_INIT;
		reshaping_done = true;
		return true;
	}

	iPoint map_pos = App->map->WorldToMap(unit->position.x, unit->position.y);

	if(App->pathfinding->CreatePath(map_pos, destination) <= 0)  Stop();  
	else
	{
		const std::list<iPoint> commander_path = *App->pathfinding->GetLastPath();
		if (!ProcessPath(commander_path)) Stop(); 
	}


	return true;
}

bool MoveToSquad::OnUpdate(float dt)
{
	bool all_idle = true;

	for (int i = 0; i < squad->units.size(); i++)
		if (!squad->units[i]->commands.empty()) all_idle = false;
	
	if (all_idle) { squad->commands.push_front(new ReshapeSquad(squad->commander)); Stop(); }

	return true;
}


bool MoveToSquad::ProcessPath(const std::list<iPoint>& path)
{
	std::vector<std::list<iPoint*>*> paths_list;

	for (int i = 0; i < squad->units.size(); i++)
	{
		std::list<iPoint*>* new_path = new std::list<iPoint*>();
		for (std::list<iPoint>::const_iterator it = path.begin(); it != path.end(); it++)
		{
			if (*it - *std::prev(it) == *std::next(it) - *it)
				continue;

			iPoint* offset_pos = new iPoint((*it) + squad->unit_offset[i]);
			if (App->pathfinding->IsWalkable(*offset_pos)) new_path->push_back(offset_pos);
			else
			{
				if(new_path->empty()) new_path->push_back(new iPoint(*it));
				else
				{
					if ((*it).DistanceManhattan(*new_path->back()) == 1) new_path->push_back(new iPoint(*it));
					else
					{
						iPoint new_point = *it;
						if (App->pathfinding->CreatePath(*new_path->back(), new_point) < 0)
							return false;
						else {
							const std::list<iPoint>* repathing = App->pathfinding->GetLastPath();
							for (std::list<iPoint>::const_iterator it2 = repathing->begin(); it2 != repathing->end(); it2++)
								new_path->push_back(new iPoint(*it2));
						}
					}
				}
			}
		}
		paths_list.push_back(new_path);
	}

	for (int i = 0; i < squad->units.size(); i++)
	{
		MoveTo* new_move_order = new MoveTo(squad->units[i], destination);

		for (std::list<iPoint*>::iterator it = paths_list[i]->begin(); it != paths_list[i]->end(); it++)
			new_move_order->path.push_back(*(*it));

		new_move_order->state = UPDATE;
		squad->units[i]->commands.push_back(new_move_order);
	}

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
