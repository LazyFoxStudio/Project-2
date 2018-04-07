#pragma once

#ifndef _HERO_H_
#define _HERO_H_

#include "Unit.h"
#include "Skills.h"
#include "j1Timer.h"

class Hero : public Unit
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