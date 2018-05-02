#pragma once
#ifndef _COMMAND_H_
#define _COMMAND_H_

#define PROXIMITY_FACTOR 2  // the higher the sooner units will reach destination (in tiles)  // 1 ~ 5//
#define ATK_PROXIMITY_FACTOR 30  // the higher the further the units will approach their enemies (in pixels)
#define MAX_NEXT_STEP_MODULE 25.0f   // max value for the next_step vector, for steering calculations  // 10 ~ 50//

#include <list>
#include <vector>
#include "p2Point.h"
#include "j1Timer.h"

class Unit;
class Squad;
class FlowField;
class Entity;

enum Command_State { TO_INIT, UPDATE, TO_STOP, FINISHED };
enum Command_Type { NOTHING, MOVETO, ATTACKING_MOVETO, ATTACK, PATROL, HOLD, MOVETOSQUAD, ATTACKING_MOVETO_SQUAD, ATTACK_SQUAD};

//  BASE CLASSES: =======================

class Command
{
public:
	Command_State state = TO_INIT;
	Unit* unit			= nullptr;
	Command_Type type	= NOTHING;

public:
	Command(Unit* unit, Command_Type type) : unit(unit), type(type) {};
	virtual ~Command() { OnStop(); };

	void Execute(float dt);
	void Stop();
	void Restart();

private:
	virtual bool OnInit()			{ return false; };
	virtual bool OnUpdate(float dt) { return false; };
	virtual bool OnStop()			{ return false; };

};


//		UNITS:  ===================================

class MoveTo : public Command
{
public:
	iPoint dest = { 0,0 };
	iPoint map_p = { 0,0 };
	FlowField* flow_field = nullptr;

public:
	MoveTo(Unit* unit, iPoint destination, FlowField* flow_field) : Command(unit, MOVETO), dest(destination), flow_field(flow_field) {};

private:
	bool OnInit();
	virtual bool OnUpdate(float dt);
	bool OnStop();

};

class Attack : public Command
{
public:

	iPoint map_p = { 0,0 };
	int current_target = -1;

	j1Timer timer;
	std::list<iPoint> path;
	std::vector<uint>* enemy_ids = nullptr;

public:
	Attack(Unit* unit, std::vector<uint>* enemy_ids) : Command(unit, ATTACK), enemy_ids(enemy_ids) {};

private:
	bool OnInit();
	bool OnUpdate(float dt);
	bool OnStop();

	bool searchTarget();
	void moveToTarget();
	void callRetaliation(Entity* enemy);

};

//		SQUADS: =======================


class MoveToSquad : public Command
{
public: 
	Squad* squad = nullptr;
	iPoint dest = { -1,-1 };
	FlowField* flow_field = nullptr;

public:

	MoveToSquad(Unit* commander, iPoint map_dest);

	bool allIdle();
private:

	bool OnInit();
	virtual bool OnUpdate(float dt);
	bool OnStop();

};

class AttackingMoveToSquad : public MoveToSquad
{
	std::vector<uint> enemy_ids;
	int target_squad_id = -1;
	bool hold = false;
	j1Timer timer;

public:
	AttackingMoveToSquad(Unit* commander, iPoint map_dest, bool hold = false, int target_squad_id = -1) : MoveToSquad(commander, map_dest), hold(hold), target_squad_id(target_squad_id) 
		{ type = ATTACKING_MOVETO_SQUAD; };

private:
	bool OnUpdate(float dt);

	bool checkSquadTarget();
};

class PatrolSquad : public Command
{
public:
	Squad * squad = nullptr;
	std::list<iPoint> patrol_points;

public:
	PatrolSquad(Unit* unit, std::list<iPoint>& patrol_points) : Command(unit, PATROL), patrol_points(patrol_points) {};
	~PatrolSquad() { patrol_points.clear(); }
	
private:

	bool OnInit() { return true; };
	bool OnUpdate(float dt);
	bool OnStop() { return true; };
};


#endif
