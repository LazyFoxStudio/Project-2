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

	float max_speed = 0.0f;

public:
	Squad() {};
	Squad(std::vector<Unit*>& units);
	~Squad();

	int getTotalHP();
	int getTotalMaxHP();
	int getUnitPriority(Unit* unit);
	Unit* getClosestUnitTo(iPoint map_p);

	void Halt();
	bool Update(float dt);

};


#endif