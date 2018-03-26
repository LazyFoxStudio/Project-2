#include "Building.h"

Building::Building(iPoint pos, buildingType type) : type(type)
{
	position.x = pos.x, position.y = pos.y;
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