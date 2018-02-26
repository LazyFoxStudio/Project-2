#pragma once

#ifndef _UNIT_H_
#define _UNIT_H_

#include "Entity.h"
#include "Effects.h"
#include "p2Animation.h"
#include "Squad.h"
#include <list>
#include <vector>

enum unitType
{
	NONE_UNIT,
	DEFENDER,
	//...

};

class Unit : public Entity
{
public:
	//Stats
	unitType type = NONE_UNIT;
	int current_HP = 0;
	int max_HP = 0;
	int attack = 0;
	int defense = 0;
	//...

	//Utilities
	std::list<iPoint> path;
	Squad* squad = nullptr;
	Entity* target = nullptr;
	bool flying = false;

	std::vector<Animation*> animations;
	std::vector<Effect*> effects;
	Animation* current_anim = nullptr;

public:
	Unit(iPoint pos, unitType type, Squad* squad = nullptr);
	
	virtual Entity* findTarget() { return nullptr; };
	virtual void attackEntity(Entity* target) {};

	void Draw();
	void Move(fPoint direction);
	void animationController();
};

#endif