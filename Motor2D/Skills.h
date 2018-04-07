
#ifndef _SKILLS_H_
#define _SKILLS_H_

#include "j1Timer.h"
#include "p2Point.h"

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
	
	bool				ready = true;
	
	int					cooldown = 0; 
	int					damage = 0;
	
	uint				radius = 0;
	uint				range = 0;
	
	iPoint				position = { 0,0 };
	iPoint				mouse_position = { 0,0 };
	iPoint				position_hero = { 0,0 };

	rangeType			type=NONE_RANGE;
	
	std::list<iPoint>	toDraw;

public:
	
	Skill(uint _radius, int _damage,uint _range,rangeType _type) 
	{
		radius = _radius;
		damage = _damage;
		range = _range;
		type = _type;

		cooldown = 5;
	};
	
	~Skill() {};

	/*virtual void Activate(Hero* hero) {};
	virtual void DrawRange() {};*/

	void Activate(Hero* hero);
	void DrawRange();
	void BFS();
	void Line();
	void MakeDamage();
	
	bool Find(std::list<iPoint> list,const iPoint& point);
	bool inCircle(int pos_x,int pos_y);
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