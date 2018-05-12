#ifndef _EFFECTS_H_
#define _EFFECTS_H_

#include "j1Timer.h"

class Unit;

enum operation_sign
{
	PLUS_MINUS,//means you want to add or subtract a number(+2,-2)
	MULTIPLICATION_DIVISION,//means you want to multiply the value(x2,x1/2)
	OTHER,//maybe its another kind of buff idk(none of these is in the code)
	END
};

enum stat_affected
{
	ATTACK_STAT,
	PIERCING_ATK_STAT,
	DEFENSE_STAT,
	SPEED_STAT,
	RANGE_STAT,
	NOSTAT
};

class Effect
{
public:
	float buff = 0;
	float elapsed = 0.0f;
	float duration = 0;
	j1Timer timer;
	Unit* unit = nullptr;
	operation_sign sign;
	bool applied = false;

	stat_affected type = NOSTAT;

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

public:
	DamageBuff(int _buff, int duration, Unit* unit, operation_sign sign_) : Effect(unit, duration, sign_) { type = ATTACK_STAT; buff = _buff; };
	~DamageBuff() { Remove(); };

	void Apply();
	void Remove();
};

class PiercingDamageBuff : public Effect
{
public:

public:
	PiercingDamageBuff(int _buff, int duration, Unit* unit, operation_sign sign_) : Effect(unit, duration, sign_) { type = PIERCING_ATK_STAT; buff = _buff; };
	~PiercingDamageBuff() { Remove(); };

	void Apply();
	void Remove();
};

class SpeedBuff : public Effect
{
public:
	
public:
	SpeedBuff(int _buff, int duration, Unit* unit, operation_sign sign_) : Effect(unit, duration, sign_) { type = SPEED_STAT; buff = _buff;
	};
	~SpeedBuff() { Remove(); };

	void Apply();
	void Remove();
};

class DefenseBuff : public Effect
{
public:

public:
	DefenseBuff(int _buff, int duration, Unit* unit, operation_sign sign_) : Effect(unit, duration, sign_) { type = DEFENSE_STAT; buff = _buff;
	};
	~DefenseBuff() { Remove(); };

	void Apply();
	void Remove();
};

class RangeBuff : public Effect
{
public:
	float buff = 0;

public:
	RangeBuff(int _buff, int duration, Unit* unit, operation_sign sign_) : Effect(unit, duration, sign_) { type = RANGE_STAT;  buff = _buff;
	};
	~RangeBuff() { Remove(); };

	void Apply();
	void Remove();
};


//(...)

#endif