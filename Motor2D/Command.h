#pragma once
#ifndef _COMMAND_H_
#define _COMMAND_H_

#include <list>
#include <vector>
#include "p2Point.h"
#include "j1Timer.h"

class Unit;
class Squad;
class FlowField;
class Entity;

enum Command_State { TO_INIT, UPDATE, TO_STOP, FINISHED };
enum Command_Type { NOTHING, MOVETO, ATTACKING_MOVETO, ATTACK, PATROL, MOVETOSQUAD, ATTACKING_MOVETO_SQUAD};

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

	fPoint getDesiredPlace();

};

class Attack : public Command
{
public:

	iPoint map_p = { 0,0 };
	iPoint current_target = { 0,0 };

	std::list<iPoint> path;
	std::vector<iPoint>* enemy_atk_slots = nullptr;

public:
	Attack(Unit* unit, std::vector<iPoint>* enemy_atk_slots) : Command(unit, ATTACK), enemy_atk_slots(enemy_atk_slots) {};

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
	void Launch();
	
	bool launched = false;
private:

	bool OnInit();
	virtual bool OnUpdate(float dt);
	bool OnStop();

};

class AttackingMoveToSquad : public MoveToSquad
{
	int target_squad_id = -1;
	bool hold = false;
	bool enemies_found = false;
	std::vector<iPoint> enemy_atk_slots;

public:
	AttackingMoveToSquad(Unit* commander, iPoint map_dest, bool hold = false, int target_squad_id = -1) : MoveToSquad(commander, map_dest), hold(hold), target_squad_id(target_squad_id) 
		{ type = ATTACKING_MOVETO_SQUAD;};

private:
	bool OnUpdate(float dt);

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
