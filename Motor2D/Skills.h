
#ifndef _SKILLS_H_
#define _SKILLS_H_

#include "j1Timer.h"
#include "p2Point.h"
#include "Color.h"

#include <list>

class Hero;

enum rangeType
{
	NONE_RANGE,
	AREA,
	LINE
};

class Skill
{
public:
	
	j1Timer				timer;
	uint				cooldown = 0; 

	uint				radius = 0;
	int					damage = 0;
	uint				range = 0;
	iPoint				cast_pos = { 0,0 };

	rangeType			type=NONE_RANGE;
	Color				tile_color;
	Hero*				hero = nullptr;
	
	std::list<iPoint>	toDraw;

public:
	
	Skill(Hero* hero, uint _radius, int _damage, uint _range, uint _cooldown, rangeType	type);
	~Skill() {};

	void Activate();
	void DrawRange();
	void BFS();
	void Line();

	bool Ready() { return timer.ReadSec() >= cooldown; };
};

//class Shockwave : public Skill
//{
//public:
//	int				radius = 0;
//	int				damage = 0;
//	iPoint			position = { 0,0 };
//	std::list<iPoint> toDraw;
//	
//public:
//	Shockwave(int radius, int damage) : radius(radius), damage(damage) {}
//	~Shockwave();
//
//	void Activate(Hero* hero);
//	void DrawRange();
//	void BFS(int Area);
//	void MakeDamage();
//
//	bool Find(std::list<iPoint> list,const iPoint& point);
//};

//(...)

#endif