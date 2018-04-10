#include "Building.h"
#include "j1Render.h"
#include "j1Scene.h"
#include "j1EntityController.h"

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
	additional_size.x = building.additional_size.x;
	additional_size.y = building.additional_size.y;

	for (int i = 0; i < 9; i++)
		available_actions[i] = building.available_actions[i];

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
	//minimap_
	if (App->scene->minimap != nullptr) 
	{
		SDL_Color color;
		color.r = 0;
		color.b = 255;
		color.g = 0;
		color.a = 255;
		App->scene->minimap->Addpoint({ (int)position.x,(int)position.y,100,100 }, color);
	}

	if (being_built)
	{
		HandleConstruction();
	}

	if (!destroyed && current_HP <= 0)
	{
		destroyed = true;
		timer.Start();
	}

	if (destroyed)
	{
		HandleDestruction();
	}

	if (!being_built && !destroyed && type == LUMBER_MILL)
	{
		HandleResourceProduction();
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

	else if (destroyed)
	{
		//TODO add the sprites of the destroyed buildings to the spritesheet.
		current_sprite = sprites[0];
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

void Building::HandleConstruction()
{
	if (type != TOWN_HALL)
	{
		int current_time = timer.ReadSec();
		if (current_time >= building_time)
		{
			int hp_unit = max_HP / building_time;
			current_HP += hp_unit - 1;
			being_built = false;
			last_frame_time = 0;
			App->scene->inactive_workers += 1;
			timer.Start();
			if (type == FARM)
			{
				App->scene->workers += 5;
				App->scene->inactive_workers += 5;
			}
		}
		else if (current_time > last_frame_time)
		{
			last_frame_time = current_time;
			int hp_unit = max_HP / building_time;
			current_HP += hp_unit;
		}
	}

	else if (type == TOWN_HALL)
	{
		being_built = false;
		current_HP = max_HP;
	}
}

void Building::HandleDestruction()
{
	if (timer.ReadSec() > App->entitycontroller->death_time)
	{
		App->map->WalkabilityArea(position.x, position.y, size.x, size.y,true);
		
		delete this;
	}
}

void Building::HandleResourceProduction()
{
	if (timer.ReadSec() >= 5 && !App->map->WalkabilityArea(position.x - (additional_size.x * App->map->data.tile_width / 2) + collider.w / 2, (position.y - (additional_size.x * App->map->data.tile_width / 2)) + collider.h / 2, additional_size.x, additional_size.y, false, true))
	{
		timer.Start();
		App->scene->wood += 5*villagers_inside * App->entitycontroller->worker_wood_production;
	}
}

void Building::Draw(float dt)
{
	App->render->Blit(texture, position.x, position.y, &current_sprite);
}