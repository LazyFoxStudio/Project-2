#ifndef _UNIT_H_
#define _UNIT_H_

#include "Entity.h"
#include "Command.h"
#include "Effects.h"
#include <list>
#include <vector>
#include <deque>

#define SPEED_CONSTANT 100.0f   // applied to all units   
#define SQUAD_UNATTACH_DISTANCE 100.0f

#define MAX_NEXT_STEP_MULTIPLIER 1.2f
#define MIN_NEXT_STEP_MULTIPLIER 0.7f

enum direction { E, NE, SE, N, S, NW, W, SW};

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


	fPoint mov_direction = { 0.0f,0.0f };
	float mov_module = 0.0f;
	fPoint mov_target = { 0.0f, 0.0f };
	
	Animation* current_anim = nullptr;
	animationType last_anim = IDLE_S;
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

	void lookAt(fPoint direction);
	void animationController();

	void Halt();
	void Destroy();

	void calculateSeparationVector(fPoint& separation_v, float& weight);
	Command_Type getCurrentCommand() { return (commands.empty() ? NOTHING : commands.front()->type); }

	void AddDamagebuff(int duration, int amount, operation_sign sign);
	void AddPiercingDamagebuff(int duration, int amount, operation_sign sign);
	void AddSpeedbuff(int duration, int amount, operation_sign sign);
	void AddDefensebuff(int duration, int amount, operation_sign sign);
	void AddRangebuff(int duration, int amount, operation_sign sign);


};

#endif