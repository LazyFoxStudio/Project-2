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
	std::vector<uint> units_id;
	std::deque<Command*> commands;

	float max_speed = 0.0f;
	int UID = -1;

public:
	Squad(std::vector<uint>& units);
	~Squad();

	bool isInSquadSight(fPoint position);
	bool getEnemiesInSight(std::vector<uint>& list_to_fill, int target_squad_UID = -1);
	void getUnits(std::vector<Unit*>& list_to_fill);
	Unit* getCommander();
	
	void removeUnit(uint unit_id);

	void Halt();
	bool Update(float dt);
	void Destroy();

	Command_Type getCurrentCommand() { return (commands.empty() ? NOTHING : commands.front()->type); }

};


#endif