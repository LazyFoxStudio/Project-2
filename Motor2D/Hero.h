#pragma once

#ifndef _HERO_H_
#define _HERO_H_

#include "Unit.h"
#include "Skills.h"
#include "j1Timer.h"

#define HERO_REVIVE_COOLDOWN 60

class Hero : public Unit
{
public:
	int current_level = 1;
	uint XP = 0;
	uint XP_to_levelup = 0;

	Skill* skill_one = nullptr;
	Skill* skill_two = nullptr;
	Skill* skill_three = nullptr;

	uint current_skill = 0;
	j1Timer revive_timer;

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