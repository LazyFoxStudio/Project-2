#pragma once

#ifndef _SQUAD_H_
#define _SQUAD_H_

class Unit;
class Entity;
class Command;

#include "p2Point.h"
#include <list>
#include <vector>
#include <deque>

class Squad
{
public:
	Unit* commander = nullptr;       // commander MUST be units[0]
	std::vector<Unit*> units;
	std::deque<Command*> commands;
	std::vector<iPoint> unit_offset;

public:
	Squad(std::vector<Unit*>& units);
	~Squad();

	int getTotalHP();
	int getTotalMaxHP();
	int getUnitPriority(Unit* unit);

	void Halt();
	void generateOffsets();
	bool Update(float dt);

};


#endif