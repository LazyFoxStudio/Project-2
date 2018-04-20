#pragma once

#ifndef _SQUAD_H_
#define _SQUAD_H_

class Unit;
class Entity;

#include "p2Point.h"
#include "Command.h"
#include <list>
#include <vector>
#include <deque>

class Squad
{
public:
	Unit* commander = nullptr;     
	std::vector<Unit*> units;
	std::deque<Command*> commands;

	float max_speed = 0.0f;

public:
	Squad(std::vector<Unit*>& units);
	~Squad();

	Unit* getClosestUnitTo(iPoint map_p);
	bool isInSquadSight(fPoint position);
	bool getEnemiesInSight(std::list<fPoint>& list_to_fill);

	void removeUnit(Unit* unit);

	void Halt();
	bool Update(float dt);

	Command_Type getCurrentCommand() { return (commands.empty() ? NOTHING : commands.front()->type); }

};


#endif