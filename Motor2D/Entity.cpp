
#include "j1EntityController.h"
#include "Entity.h"
#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "Entity.h"

iPoint Entity::getSize()
{
	return iPoint(0, 0);
}

void Entity::selectedDraw()
{

}

bool Entity::Collides(Entity& other)
{
	return false;
}

