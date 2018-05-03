#ifndef _UNIT_H_
#define _UNIT_H_

#include "Entity.h"
#include "Command.h"
#include "Effects.h"
#include <list>
#include <vector>
#include <deque>

enum direction { E, NE, SE, N, S, NW, W, SW};

#define SPEED_CONSTANT 1.2f   // applied to all units   
#define MAX_MOVEMENT_WEIGHT 100.0f   // max value for the next_step vector, for steering calculations  /
#define STEERING_FACTOR 1.5f


class Animation;
class Squad;
//class Effect;

class Unit : public Entity
{
public:
	//Stats
	float speed = 0.0f;
	uint squad_members = 1;

	//Utilities
	Squad* squad = nullptr;
	bool flying = false;

	fPoint displacement = { 0.0f,0.0f };
	fPoint next_step = { 0.0f, 0.0f };
	fPoint movement_target = { 0.0f, 0.0f };
	
	Animation* current_anim = nullptr;
	direction dir = S;
	
	std::vector<Animation*> animations;
	std::list<Effect*> effects;
	std::deque<Command*> commands;

public:
	Unit() {};
	Unit(iPoint pos, Unit& unit, Squad* squad = nullptr);
	~Unit();

	bool Update(float dt);
	void Draw(float dt);
	void Move(float dt);
	void SquadMove(float dt);

	void lookAt(fPoint direction);
	void animationController();

	void Halt();
	void Destroy();

	void calculateSeparationVector(fPoint& separation_v, int& weight);
	Command_Type getCurrentCommand() { return (commands.empty() ? NOTHING : commands.front()->type); }

	void AddDamagebuff(int duration, int amount, operation_sign sign);
	void AddPiercingDamagebuff(int duration, int amount, operation_sign sign);
	void AddSpeedbuff(int duration, int amount, operation_sign sign);
	void AddDefensebuff(int duration, int amount, operation_sign sign);
	void AddRangebuff(int duration, int amount, operation_sign sign);


};

#endif