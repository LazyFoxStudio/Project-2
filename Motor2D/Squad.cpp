#include "Squad.h"
#include "Unit.h"
#include "Command.h"

Squad::Squad(std::vector<Unit*>& units) : units(units)
{
	generateOffsets();
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
	unit_offset.clear();
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

void Squad::generateOffsets()
{
	int count = 1;
	int radius = 1;

	unit_offset.push_back(iPoint{ 0,0 });

	while (radius < 4)
	{
		for (int i = -radius; i <= radius; i++)
			for (int j = -radius; j <= radius; j++)
				if (std::abs(i) == radius || std::abs(j) == radius)
				{
					unit_offset.push_back(iPoint(i, j));
					count++;
					if (count == units.size()) return;
				}
		radius++;
	}

}

int Squad::getUnitPriority(Unit* unit)
{
	for (int i = 0; i < units.size(); i++)
		if (unit = units[i]) return i;

	return -1;
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
