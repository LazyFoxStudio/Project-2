#include "Building.h"
#include "j1Render.h"

Building::Building(iPoint pos, Building& building)
{
	name							= building.name;
	texture							= building.texture;
	type							= building.type;

	current_HP						= building.current_HP;
	max_HP							= building.max_HP;
	villagers_inside				= building.villagers_inside;
	cooldown_time					= building.cooldown_time;
	defense							= building.defense;
	
	entity_type						= BUILDING;
	position.x = pos.x, position.y	= pos.y;

	sprites = building.sprites;

}

Building::~Building()
{

}

bool Building::Update(float dt)
{
	if (being_built)
	{
		current_sprite = sprites[0];
	}
	if (!being_built && type != TOWN_HALL)
	{
		current_sprite = sprites[1];
	}
	return true;
}

void Building::Draw(float dt)
{
	App->render->Blit(texture, position.x, position.y, &current_sprite);
}