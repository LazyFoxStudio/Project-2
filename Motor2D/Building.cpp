#include "Building.h"

Building::Building(iPoint pos, Building& building)
{
	name							= building.name;
	texture							= building.texture;
	type							= building.type;

	current_HP						= building.current_HP;
	max_HP							= building.max_HP;
	villagers_inside				= building.villagers_inside;
	cooldown						= building.cooldown;
	
	entity_type						= BUILDING;
	position.x = pos.x, position.y	= pos.y;
}

Building::~Building()
{

}

bool Building::Update(float dt)
{
	return true;
}

void Building::Draw()
{

}