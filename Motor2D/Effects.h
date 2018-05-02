#ifndef _EFFECTS_H_
#define _EFFECTS_H_

#include "SDL\include\SDL_timer.h"

class Unit;

enum operation_sign
{
	PLUS_MINUS,
	MULTIPLICATION_DIVISION,
	OTHER,//maybe its another number
	END
};

class Effect
{
public:
	float elapsed = 0.0f;
	int duration = 0;
	float started_at = 0;
	Unit* unit = nullptr;
	operation_sign sign;
	bool applied = false;

public:
	Effect(Unit* unit, int duration, operation_sign sign_) : unit(unit), duration(duration), sign(sign_) { started_at = SDL_GetTicks(); };
	~Effect() {};

	//void updateTimer(float dt) {elapsed += dt;};
	virtual void Apply() {};
	virtual void Remove() {};
};

class DamageBuff : public Effect
{
public:
	int buff = 0;

public:
	DamageBuff(int buff, int duration, Unit* unit, operation_sign sign_) : buff(buff), Effect(unit, duration,sign_) {};
	~DamageBuff() { Remove(); };

	void Apply();
	void Remove();
};

//(...)

#endif