#pragma once
#ifndef _NATURE_H_
#define _NATURE_H_

#include "Entity.h"



class Nature : public Entity
{
public:
	resourceType type = NONE_NATURE;
	int amount = 0;

public:
	Nature() { entity_type = NATURE; };
	Nature(iPoint pos, Nature& nature, int amount = 0);
	~Nature();

	void Draw();
};


#endif