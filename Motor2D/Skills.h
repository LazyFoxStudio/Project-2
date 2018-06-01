
#ifndef _SKILLS_H_
#define _SKILLS_H_

#include "j1Timer.h"
#include "p2Point.h"
#include "Color.h"
#include "SDL/include/SDL.h"

#include <list>

class Hero;

enum rangeType
{
	NONE_RANGE,
	AREA,
	LINE,
	PLACE,
	HEAL,
	BUFF
};

class Skill
{
public:
	
	j1Timer				timer;
	uint				cooldown = 0; 

	uint				radius = 0;
	int					damage = 0;
	int					upgrade = 0;
	uint				range = 0;
	iPoint				cast_pos = { 0,0 };
	iPoint				last_cast = { 0,0 };

	rangeType			type=NONE_RANGE;
	Color				tile_color;
	Hero*				hero = nullptr;

	SDL_Rect			text_rec = { 0,0,0,0 };

	bool				going = false;
	
	std::list<iPoint>	toDraw;

public:
	
	Skill(Hero* hero, uint _radius, int _damage, int _upgrade, uint _range, uint _cooldown, rangeType	type);
	~Skill() {};

	bool Activate();
	void DrawRange();
	void BFS();
	void Line();

	int GetUpgrade();

	bool Ready() { return timer.ReadSec() >= cooldown; };
};

#endif