#include "Nature.h"

Nature::Nature(iPoint pos, Nature& nature, int amount) : amount(amount)
{
	name							= nature.name;
	texture							= nature.texture;
	type							= nature.type;

	entity_type						= NATURE;
	position.x = pos.x, position.y	= pos.y;
}

Nature::~Nature()
{

}

void Nature::Draw()
{

}