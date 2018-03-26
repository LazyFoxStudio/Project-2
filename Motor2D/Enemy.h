#ifndef __ENEMY_H__
#define __ENEMY_H__

#include "Unit.h"

class Enemy : public Unit
{
public:

public:
	~Enemy();
	Entity* findTarget();
	void attackEntity(Entity* target);
};

#endif
