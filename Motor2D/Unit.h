#ifndef _UNIT_H_
#define _UNIT_H_

#include "Entity.h"
#include "Command.h"
#include "Effects.h"
#include <list>
#include <vector>
#include <deque>

enum direction { E, NE, SE, N, S, NW, W, SW};

#define COLLIDER_MARGIN 5  // extra margin for separation calculations  // 10 ~ 30//

#define STEERING_FACTOR 5.0f    // the higher the stiffer      // 4.0f ~ 10.0f//

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
	fPoint movement = { 0.0f,0.0f };
	fPoint next_step = { 0.0f, 0.0f };
	
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

	void lookAt(fPoint direction);
	void animationController();

	void Halt();
	void Destroy();

	fPoint calculateSeparationVector();
	fPoint calculateCohesionVector();
	fPoint calculateAlignementVector();
	Command_Type getCurrentCommand() { return (commands.empty() ? NOTHING : commands.front()->type); }

	void AddDamagebuff(int duration, int amount, operation_sign sign);
	void AddPiercingDamagebuff(int duration, int amount, operation_sign sign);
	void AddSpeedbuff(int duration, int amount, operation_sign sign);
	void AddDefensebuff(int duration, int amount, operation_sign sign);
	void AddRangebuff(int duration, int amount, operation_sign sign);


};

#endif