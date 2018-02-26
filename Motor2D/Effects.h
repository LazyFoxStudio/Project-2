#pragma once

#pragma once

#ifndef _EFFECTS_H_
#define _EFFECTS_H_

#include "j1Timer.h"

class Unit;

class Effect
{
public:
	j1Timer timer;
	int duration = 0;
	Unit* unit = nullptr;

public:
	Effect(Unit* unit) : unit(unit) {};
	~Effect() {};

	void updateTimer() {};
	virtual void Apply(Unit* unit) {};
	virtual void Remove(Unit* unit) {};
};


class DamageBuff : public Effect
{
public:
	int buff = 0;

public:
	DamageBuff(int buff, Unit* unit) : buff(buff), Effect(unit) {};
	~DamageBuff() { Remove(); };

	void Apply();
	void Remove();
};

//(...)

#endif