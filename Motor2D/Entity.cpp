
#include "j1EntityController.h"
#include "Entity.h"
#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "Entity.h"

int Entity::getHP()
{
	if		(entity_type == UNIT)		return ((Unit*)this)->current_HP;
	else if (entity_type == BUILDING)	return ((Building*)this)->current_HP;
	else return 0;
}

