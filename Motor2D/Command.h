#pragma once
#ifndef _COMMAND_H_
#define _COMMAND_H_

#define STEERING_FACTOR 7.0f    // the higher the stiffer      // 4.0f ~ 10.0f//
#define PROXIMITY_FACTOR 2  // the higher the sooner units will reach destination  // 1 ~ 5//

#include <list>
#include <vector>
#include "p2Point.h"
#include "j1Timer.h"

class Unit;
class Squad;
class FlowField;

enum Command_State { TO_INIT, UPDATE, TO_STOP, FINISHED };
enum Command_Type { NONE, MOVETO, ATTACKING_MOVETO, ATTACK, PATROL, HOLD, MOVETOSQUAD, ATTACKING_MOVETO_SQUAD, ATTACK_SQUAD};

//  BASE CLASSES: =======================

class Command
{
public:
	Command_State state = TO_INIT;
	Unit* unit			= nullptr;
	Command_Type type	= NONE;

public:
	Command(Unit* unit, Command_Type type) : unit(unit), type(type) {};
	virtual ~Command() {};

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
	FlowField* flow_field = nullptr;
	std::list<fPoint>* enemy_positions = nullptr;

public:
	Attack(Unit* unit, FlowField* flow_field, std::list<fPoint>* enemy_positions) : Command(unit, ATTACK), flow_field(flow_field), enemy_positions(enemy_positions) {};

private:
	bool OnInit();
	bool OnUpdate(float dt);
	bool OnStop();

};

//
//class Hold : public Command
//{
//public:
//	iPoint held_position = { 0,0 };
//
//public:
//	Hold(Unit* unit, iPoint position) : Command(unit, HOLD), held_position(position) {};
//
//private:
//
//	bool OnInit();
//	bool OnUpdate(float dt);
//	bool OnStop() { return true; };
//};
//
//
//class Patrol : public Command
//{
//public:
//	std::vector<iPoint> patrol_points;
//	uint current_point = 0;
//
//public:
//	Patrol(Unit* unit, std::vector<iPoint>& patrol_points) : Command(unit, PATROL), patrol_points(patrol_points) {};
//	~Patrol() { patrol_points.clear(); }
//	
//private:
//
//	bool OnInit();
//	bool OnUpdate(float dt);
//	bool OnStop() { return true; };
//};


//		SQUADS: =======================


class MoveToSquad : public Command
{
public: 
	Squad* squad = nullptr;
	iPoint dest = { -1,-1 };
	FlowField* flow_field = nullptr;

public:

	MoveToSquad(Unit* commander, iPoint map_dest) : Command(commander, MOVETOSQUAD), dest(map_dest) {};

private:

	bool OnInit();
	virtual bool OnUpdate(float dt);
	virtual bool OnStop();

};

class AttackingMoveToSquad : public MoveToSquad
{
	FlowField* atk_flow_field = nullptr;
	std::list<fPoint> enemy_positions;
	bool enemies_in_sight = false;

public:
	AttackingMoveToSquad(Unit* commander, iPoint map_dest) : MoveToSquad(commander, map_dest) { type = ATTACKING_MOVETO_SQUAD; };

private:
	bool OnUpdate(float dt);
	bool OnStop();
};

#endif
