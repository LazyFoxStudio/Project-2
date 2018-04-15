#include "Squad.h"
#include "Unit.h"
#include "Command.h"
#include "j1EntityController.h"

#include "j1Input.h"
#include "j1Render.h"

Squad::Squad(std::vector<Unit*>& units) : units(units)
{
	commander = units[0];

	max_speed = units[0]->speed;

	for (int i = 0; i < units.size(); i++)
	{
		units[i]->squad = this;
		if (units[i]->speed < max_speed) max_speed = units[i]->speed < max_speed;
	}
}

Squad::~Squad()
{
	commands.clear();
}

bool Squad::Update(float dt)
{
	if (!commands.empty())
	{
		commands.front()->Execute(dt);
		if (commands.front()->state == FINISHED) commands.pop_front();
	}

	return true;
}

void Squad::removeUnit(Unit* unit)
{
	for (std::vector<Unit*>::iterator it = units.begin(); it != units.end(); it++)
		if ((*it) == unit) { units.erase(it); RELEASE (unit); return; }
}

bool Squad::isInSquadSight(fPoint position)
{
	for (int i = 0; i < units.size(); i++)
		if (position.DistanceTo(units[i]->position) < units[i]->line_of_sight) return true;

	return false;
}

bool Squad::getEnemiesInSight(std::list<fPoint>& list_to_fill)
{
	list_to_fill.clear();

	for (std::list<Entity*>::iterator it = App->entitycontroller->entities.begin(); it != App->entitycontroller->entities.end(); it++)
	{
		if ((*it)->isActive == false) continue;

		if (((*it)->entity_type == BUILDING && commander->IsEnemy()))
		{
			if (isInSquadSight((*it)->position) && ((Building*)(*it))->ex_state != DESTROYED)
				list_to_fill.push_back((*it)->position);
		}
		else if ((*it)->entity_type == UNIT)
		{
			if (isInSquadSight((*it)->position) && ((Unit*)(*it))->IsEnemy() != commander->IsEnemy())
				list_to_fill.push_back((*it)->position);
		}
	}
	return !list_to_fill.empty();
}


void Squad::Halt()
{
	for (int i = 0; i < units.size(); i++)
		units[i]->Halt();

	for (std::deque<Command*>::iterator it = commands.begin(); it != commands.end(); it++)
		(*it)->Restart();  // Restarting the order calls onStop(), which would be otherwise unaccesible

	commands.clear();
}

int Squad::getTotalMaxHP()
{
	int ret = 0;

	for (int i = 0; i < units.size(); i++)
		ret += units[i]->max_HP;

	return ret;
}

int Squad::getTotalHP()
{
	int ret = 0;

	for (int i = 0; i < units.size(); i++)
		ret += units[i]->current_HP;

	return ret;
}

Unit* Squad::getClosestUnitTo(iPoint p)
{
	if (units.empty() || units[0] == nullptr) return nullptr;

	Unit* ret = units[0];
	iPoint closest_map_p = App->map->WorldToMap(units[0]->position.x, units[0]->position.y);

	for (int i = 1; i < units.size(); i++)
	{
		iPoint map_p = App->map->WorldToMap(units[i]->position.x, units[i]->position.y);
		if (map_p.DistanceTo(p) < closest_map_p.DistanceTo(p))
		{
			ret = units[i];
			closest_map_p = map_p;
		}
	}
	return ret;
}
