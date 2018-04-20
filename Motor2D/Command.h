#pragma once
#ifndef _COMMAND_H_
#define _COMMAND_H_

#define STEERING_FACTOR 7.0f    // the higher the stiffer      // 4.0f ~ 10.0f//
#define PROXIMITY_FACTOR 2  // the higher the sooner units will reach destination (in tiles)  // 1 ~ 5//
#define ATK_PROXIMITY_FACTOR 30  // the higher the further the units will approach their enemies (in pixels)
#define MAX_NEXT_STEP_MODULE 50.0f   // max value for the next_step vector, for steering calculations  // 10 ~ 50//

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
	fPoint current_target = { 0.0f, 0.0f };

	j1Timer timer;
	std::list<iPoint> path;
	std::list<fPoint>* enemy_positions = nullptr;

public:
	Attack(Unit* unit, std::list<fPoint>* enemy_positions) : Command(unit, ATTACK), enemy_positions(enemy_positions) {};

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
	virtual bool OnStop();

};

class AttackingMoveToSquad : public MoveToSquad
{
	std::list<fPoint> enemy_positions;
	bool enemies_in_sight = false;
	bool hold = false;
	bool just_attacked = false;
	j1Timer timer;

public:
	AttackingMoveToSquad(Unit* commander, iPoint map_dest, bool hold = false) : MoveToSquad(commander, map_dest), hold(hold) { type = ATTACKING_MOVETO_SQUAD; };

private:
	bool OnUpdate(float dt);
	bool OnStop();
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
