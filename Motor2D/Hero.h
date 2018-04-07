#pragma once

#ifndef _HERO_H_
#define _HERO_H_

#include "Unit.h"
#include "Skills.h"
#include "j1Timer.h"

enum heroType
{
	NONE_TYPE,
	MAGE,
	PALADIN,
};

class Hero : public Entity
{
public:
	int current_level = 1;
	uint XP = 0;
	uint XP_to_levelup = 0;

	Skill* skill_one = nullptr;
	j1Timer cooldown_one;

	Skill* skill_two = nullptr;
	j1Timer cooldown_two;

	Skill* skill_three = nullptr;
	j1Timer cooldown_three;

	heroType type = NONE_TYPE;
	uint current_HP = 0;
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

	std::vector<Animation*> animations;
	std::vector<Effect*> effects;
	std::deque<Command*> commands;
	Animation* current_anim = nullptr;

	animationType new_animation = IDLE_S;

private:

	bool	doSkill_1 = false;
	bool	doSkill_2 = false;
	bool	doSkill_3 = false;

public:

	Hero() {};
	Hero(iPoint pos, Hero& unit);

	~Hero();

	bool Update(float dt);

	void UseSkill(int index);

	Entity* findTarget();
	void attackEntity(Entity* target);
};

#endif