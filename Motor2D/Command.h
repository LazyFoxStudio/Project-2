#pragma once

#ifndef _COMMAND_H_
#define _COMMAND_H_

#include <list>
#include <vector>
#include "p2Point.h"

class Unit;
class Squad;

enum Command_State { TO_INIT, UPDATE, TO_STOP, FINISHED };
enum Command_Type { NONE, MOVETO, ATTACKING_MOVETO, ATTACK, PATROL, HOLD, MOVETOSQUAD, RESHAPE_SQUAD};

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
	fPoint next_step = { 0.0f, 0.0f };
	bool waiting = false;
	std::list<iPoint> path;

public:
	MoveTo(Unit* unit, iPoint destination) : Command(unit, MOVETO), dest(destination) {};

private:
	bool OnInit();
	virtual bool OnUpdate(float dt);
	bool OnStop();

protected:
	void Repath();

};


class AttackingMoveTo : public MoveTo
{
public:
	iPoint dest = { 0,0 };
	std::list<iPoint> path;

public:
	AttackingMoveTo(Unit* unit, iPoint destination) : MoveTo(unit, destination) { type = ATTACKING_MOVETO; };

private:
	bool OnUpdate(float dt);
};


class Attack : public Command
{
public:
	bool keep_attacking = true;
	iPoint enemy_position = { 0,0 };

public:
	Attack(Unit* unit, bool keep_attacking = true) : Command(unit, ATTACK), keep_attacking(keep_attacking) {};

private:
	bool OnInit();
	bool OnUpdate(float dt);
	bool OnStop();

};


class Hold : public Command
{
public:
	iPoint held_position = { 0,0 };

public:
	Hold(Unit* unit, iPoint position) : Command(unit, HOLD), held_position(position) {};

private:

	bool OnInit();
	bool OnUpdate(float dt);
	bool OnStop() { return true; };
};


class Patrol : public Command
{
public:
	std::vector<iPoint> patrol_points;
	uint current_point = 0;

public:
	Patrol(Unit* unit, std::vector<iPoint>& patrol_points) : Command(unit, PATROL), patrol_points(patrol_points) {};
	~Patrol() { patrol_points.clear(); }
	
private:

	bool OnInit();
	bool OnUpdate(float dt);
	bool OnStop() { return true; };
};


//		SQUADS: =======================


class MoveToSquad : public Command
{
public: 
	Squad* squad = nullptr;
	bool reshaping_done = false;
	iPoint destination = { -1,-1 };

public:

	MoveToSquad(Unit* commander, iPoint map_dest) : Command(commander, MOVETOSQUAD), destination(map_dest) {};

private:

	bool ProcessPath(const std::list<iPoint>& path);
	bool OnInit();
	bool OnUpdate(float dt);
	bool OnStop() { return true; };
};


class ReshapeSquad : public Command
{
public:
	Squad * squad = nullptr;
public:

	ReshapeSquad(Unit* commander) : Command(commander, RESHAPE_SQUAD) {};

private:

	bool OnInit();
	bool OnUpdate(float dt);
	bool OnStop() { return true; };
};


#endif
