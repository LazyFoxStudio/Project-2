#include "Squad.h"
#include "Unit.h"
#include "Command.h"

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

	if (App->input->GetKey(SDL_SCANCODE_K) == KEY_DOWN)
	{
		int x = 0; int y = 0;
		App->input->GetMousePosition(x, y);
		iPoint p = App->render->ScreenToWorld(x, y);
		p = App->map->WorldToMap(p.x, p.y);
		Halt();
 		commands.push_back(new AttackingMoveToSquad(commander, p));
	}
	return true;
}

bool Squad::isInSquadSight(fPoint position)
{
	for (int i = 0; i < units.size(); i++)
		if (position.DistanceTo(units[i]->position) < units[i]->line_of_sight) return true;

	return false;
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
