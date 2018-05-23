#pragma once

#ifndef _HERO_H_
#define _HERO_H_

#include "Unit.h"
#include "Skills.h"
#include "j1Timer.h"

#define HERO_REVIVE_COOLDOWN 10
#define UNLOCK_ABILITY_1_WAVE 3
#define UNLOCK_ABILITY_2_WAVE 6

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

public:

	~Hero();

	bool Update(float dt);
	void Destroy();
	void Reactivate();

};

#endif