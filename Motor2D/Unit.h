#ifndef _UNIT_H_
#define _UNIT_H_

#include "Entity.h"
#include "Command.h"
#include "Effects.h"
#include <list>
#include <vector>
#include <deque>

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
	bool flying = false;
	fPoint next_step = { 0.0f,0.0f };
	
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
	Command_Type getCurrentCommand() { return (commands.empty() ? NOTHING : commands.front()->type); }

	void AddDamagebuff(int duration,int amount, operation_sign sign)
	{
		Effect*  ret = new DamageBuff(duration, amount, this, sign);
		switch (sign)
		{
		case PLUS_MINUS:
			{
				effects.push_front(ret);
				break; 
			}
		case MULTIPLICATION_DIVISION:
			{
				effects.push_back(ret);
				break;
			}
		default:
			{
				effects.push_back(ret);
				break;
			}
		}

	}
};

#endif