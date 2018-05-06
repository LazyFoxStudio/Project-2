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

enum Formation{ SQUARE};

class Squad
{
public:
	std::vector<uint> units_id;
	std::vector<fPoint> units_offsets;
	std::list<iPoint> atk_slots;
	std::deque<Command*> commands;
	Formation formation = SQUARE;
	fPoint commander_pos = { 0.0f,0.0f };
	float max_speed = 0.0f;
	fPoint squad_movement = { 0.0f,0.0f };
	fPoint squad_direction = { 0.0f,1.0f };
	int UID = -1;

public:
	Squad(std::vector<uint>& units);
	~Squad();

	bool isInSquadSight(fPoint position);
	bool findAttackSlots(std::vector<iPoint>& list_to_fill, int target_squad_UID = -1);
	void getUnits(std::vector<Unit*>& list_to_fill);
	Unit* getCommander();
	
	void removeUnit(uint unit_id);

	void calculateOffsets();
	fPoint getOffset(uint unit_UID);
	void calculateAttackSlots();

	void Halt();
	bool Update(float dt);
	void Destroy();

	Command_Type getCurrentCommand() { return (commands.empty() ? NOTHING : commands.front()->type); }

};


#endif