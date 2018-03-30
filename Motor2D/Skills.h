
#ifndef _SKILLS_H_
#define _SKILLS_H_

#include "j1Timer.h"
#include "p2Point.h"

#include <list>

class Hero;

enum rangeType
{
	NONE_RANGE,
	CIRCLE,
	LINEAR
};

class Skill
{
public:
	j1Timer timer;
	int cooldown = 0;
	bool ready = true;

public:
	virtual ~Skill() {};
	virtual void Activate(Hero* hero) {};
	virtual void DrawRange() {};
};


class Shockwave : public Skill
{
public:
	int				radius = 0;
	int				damage = 0;
	iPoint			position = { 0,0 };
	std::list<iPoint> toDraw;
	
public:
	Shockwave(int radius, int damage) : radius(radius), damage(damage) {}
	~Shockwave();

	void Activate(Hero* hero);
	void DrawRange();
	void BFS(int Area);

	std::list<iPoint>::iterator Find(std::list<iPoint> list,const iPoint& point);
};

//(...)

#endif