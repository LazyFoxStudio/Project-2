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
	Unit* commander = nullptr;     
	std::vector<Unit*> units;
	std::deque<Command*> commands;
	std::vector<iPoint> unit_offset;

	float max_speed = 0.0f;

public:
	Squad(std::vector<Unit*>& units);
	~Squad();

	int getTotalHP();
	int getTotalMaxHP();
	int getUnitPriority(Unit* unit);
	Unit* getClosestUnitTo(iPoint map_p);

	void Halt();
	void generateOffsets();
	bool Update(float dt);

};


#endif