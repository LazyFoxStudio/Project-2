#ifndef _EFFECTS_H_
#define _EFFECTS_H_

#include "j1Timer.h"

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
	float duration = 0;
	j1Timer timer;
	Unit* unit = nullptr;
	operation_sign sign;
	bool applied = false;

public:
	Effect(Unit* unit, int duration, operation_sign sign_) : unit(unit), duration(duration), sign(sign_) {timer.Start();};
	~Effect() {};

	//void updateTimer(float dt) {elapsed += dt;};
	virtual void Apply() {};
	virtual void Remove() {};
};

class DamageBuff : public Effect
{
public:
	float buff = 0;

public:
	DamageBuff(int buff, int duration, Unit* unit, operation_sign sign_) : buff(buff), Effect(unit, duration,sign_) {};
	~DamageBuff() { Remove(); };

	void Apply();
	void Remove();
};

class PiercingDamageBuff : public Effect
{
public:
	float buff = 0;

public:
	PiercingDamageBuff(int buff, int duration, Unit* unit, operation_sign sign_) : buff(buff), Effect(unit, duration, sign_) {};
	~PiercingDamageBuff() { Remove(); };

	void Apply();
	void Remove();
};

class SpeedBuff : public Effect
{
public:
	float buff = 0;
	
public:
	SpeedBuff(int buff, int duration, Unit* unit, operation_sign sign_) : buff(buff), Effect(unit, duration, sign_) {};
	~SpeedBuff() { Remove(); };

	void Apply();
	void Remove();
};

class DefenseBuff : public Effect
{
public:
	float buff = 0;

public:
	DefenseBuff(int buff, int duration, Unit* unit, operation_sign sign_) : buff(buff), Effect(unit, duration, sign_) {};
	~DefenseBuff() { Remove(); };

	void Apply();
	void Remove();
};

class RangeBuff : public Effect
{
public:
	float buff = 0;

public:
	RangeBuff(int buff, int duration, Unit* unit, operation_sign sign_) : buff(buff), Effect(unit, duration, sign_) {};
	~RangeBuff() { Remove(); };

	void Apply();
	void Remove();
};


//(...)

#endif