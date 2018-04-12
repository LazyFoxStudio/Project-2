#ifndef _UNIT_H_
#define _UNIT_H_

#include "Entity.h"
#include <list>
#include <vector>
#include <deque>

enum unitType
{
	NONE_UNIT,

	// Heroes
	HERO_1, /* ... */ HERO_X,
	
	// Allies
	FOOTMAN, /* ... */ ARCHER, ALLY_X,

	//Enemies
	 GRUNT, AXE_THROWER, /* ... */ ENEMY_X,
};

enum direction { E, NE, SE, N, S, NW, W, SW};

class Animation;
class Squad;
class Effect;
class Command;

class Unit : public Entity
{
public:
	//Stats
	unitType type = NONE_UNIT;
	int current_HP = 0;
	uint max_HP = 0;
	uint attack = 0;
	uint piercing_atk = 0;
	uint defense = 0;
	uint line_of_sight = 0;
	uint range = 0;
	float speed = 0.0f;
	uint gold_cost = 0;
	uint wood_cost = 0;
	uint worker_cost = 0;
	uint training_time = 0;
	uint squad_members = 1;

	//...

	//Utilities

	Squad* squad = nullptr;
	bool flying = false;
	fPoint next_step = { 0.0f,0.0f };
	
	std::vector<Animation*> animations;
	std::vector<Effect*> effects;
	std::deque<Command*> commands;
	Animation* current_anim = nullptr;

	animationType new_animation=IDLE_S;
	direction dir = S;
	

public:
	Unit() { entity_type = UNIT; };					// this constructor should only be used when loading unit Database
	Unit(iPoint pos, Unit& unit, Squad* squad = nullptr);
	~Unit();

	bool Update(float dt);
	void Draw(float dt);
	void animationController();
	void Halt();

	fPoint calculateSeparationVector();
	bool IsEnemy() { return type >= GRUNT; };
	void Move(float dt);
	void setDirection();
};

#endif