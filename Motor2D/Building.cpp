#include "Building.h"
#include "j1Render.h"
#include "j1Scene.h"

Building::Building(iPoint pos, Building& building)
{
	name							= building.name;
	texture							= building.texture;
	type							= building.type;

	current_HP						= building.current_HP;
	max_HP							= building.max_HP;
	villagers_inside				= building.villagers_inside;
	building_time					= building.building_time;
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
	if (building_timer.ReadSec() > building_time && being_built)
	{
		being_built = false;
	}

	if (being_built)
	{
		current_sprite = sprites[0];
	}

	else if (!being_built && type != TOWN_HALL)
	{
		current_sprite = sprites[1];
	}

	else if (type == TOWN_HALL)
	{
		switch (App->scene->town_hall_lvl)
		{
		case 0:
			current_sprite = sprites[1];
			break;
		case 1:
			current_sprite = sprites[2];
			break;
		case 2:
			current_sprite = sprites[3];
			break;
		}
	}
	return true;
}

void Building::Draw(float dt)
{
	App->render->Blit(texture, position.x, position.y, &current_sprite);
}