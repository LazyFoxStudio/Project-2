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
	size.x = building.size.x;
	size.y = building.size.y;

	GetColliderFromSize();

	sprites = building.sprites;

}

Building::~Building()
{

}

void Building::GetColliderFromSize()
{
	collider.x = position.x;
	collider.y = position.y;
	collider.w = size.x*App->map->data.tile_width;
	collider.h = size.y*App->map->data.tile_height;
	
}

bool Building::Update(float dt)
{
	if (being_built && type != TOWN_HALL)
	{
		int current_time = timer.ReadSec();
		if (current_time >= building_time)
		{
			int hp_unit = max_HP / building_time;
			current_HP += hp_unit -1;
			being_built = false;
			last_frame_time = 0;
		}
		else if( current_time > last_frame_time)
		{
			last_frame_time = current_time;
			int hp_unit = max_HP / building_time;
			current_HP += hp_unit;
		}
	}

	else if (being_built && type == TOWN_HALL)
	{
		being_built = false;
		current_HP = max_HP;
	}

	HandleSprite();
	
	return true;
}

void Building::HandleSprite()
{
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
}

void Building::Draw(float dt)
{
	App->render->Blit(texture, position.x, position.y, &current_sprite);
}