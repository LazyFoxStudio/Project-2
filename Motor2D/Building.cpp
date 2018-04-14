#include "Building.h"
#include "j1Render.h"
#include "j1Scene.h"
#include "j1UIScene.h"
#include "j1EntityController.h"
#include "j1Input.h"
#include "j1Scene.h"
#include "UI_Chrono.h"

Building::Building(iPoint pos, Building& building)
{
	name							= building.name;
	texture							= building.texture;
	type							= building.type;

	current_HP = max_HP				= building.max_HP;
	villagers_inside				= building.villagers_inside;
	building_time					= building.building_time;
	defense							= building.defense;
	
	entity_type						= BUILDING;

	collider.x = position.x = pos.x;
	collider.y = position.y	= pos.y;

	size.x = building.size.x;
	size.y = building.size.y;

	collider.w = size.x*App->map->data.tile_width;
	collider.h = size.y*App->map->data.tile_height;

	additional_size.x = building.additional_size.x;
	additional_size.y = building.additional_size.y;

	for (int i = 0; i < 9; i++)
		available_actions[i] = building.available_actions[i];

	sprites = building.sprites;

	if (type == TOWN_HALL)
	{
		ex_state = OPERATIVE;
		current_sprite = &sprites[1];
	}
	else current_sprite = &sprites[2];

	timer.Start();
}

Building::~Building()
{
	sprites.clear();
}


bool Building::Update(float dt)
{
	//minimap_
	if (App->uiscene->minimap != nullptr) 
	{
		SDL_Color color;
		if(ex_state != DESTROYED)	color = { 0,255,0,255 };
		else						color = { 100,100,100,255 };
		App->uiscene->minimap->Addpoint({ (int)position.x,(int)position.y,100,100 }, color);
	}

	if(current_HP <= 0 && ex_state != DESTROYED)   Destroy();

	switch (ex_state)
	{
	case BEING_BUILT:
		HandleConstruction();
		if (timer.ReadSec() >= building_time / 2 && current_sprite == &sprites[2])
			current_sprite = &sprites[0];
		break;
	case OPERATIVE:

		if (type == LUMBER_MILL)
			HandleResourceProduction();
		break;
	case DESTROYED:
		if (timer.ReadSec() > App->entitycontroller->death_time)
		{
			App->map->WalkabilityArea(position.x, position.y, size.x, size.y, true);
			App->entitycontroller->entities_to_destroy.push_back(this);
		}
	}

	return true;
}

void Building::Destroy()
{
	ex_state = DESTROYED;
	timer.Start();
	switch (type)
	{
	case FARM:
		if (App->scene->inactive_workers >= 5)
		{
			App->scene->inactive_workers -= 5;
			App->scene->workers -= 5;
		}
		current_sprite = &sprites[3];
		break;
	case LUMBER_MILL:
		App->entitycontroller->GetTotalIncome();
		current_sprite = &sprites[3];
		break;
	case TOWN_HALL:
		current_sprite = &sprites[4];
		App->gui->Chronos->counter.PauseTimer();
		App->scene->toRestart = true;
		App->scene->Restart_timer.Start();
		App->uiscene->toggleMenu(true, GAMEOVER_MENU);
		break;
	}
}


void Building::HandleConstruction()
{
	int current_time = timer.ReadSec();
	if (current_time >= building_time)
	{
		int hp_unit = max_HP / building_time;
		current_HP += hp_unit - 1;
		App->scene->inactive_workers += 1;
		timer.Start();
		if (type == FARM)
		{
			App->scene->workers += 5;
			App->scene->inactive_workers += 5;
		}
		ex_state = OPERATIVE;
		current_sprite = &sprites[1];
	}
	else if (current_time > last_frame_time)
	{
		last_frame_time = current_time;
		int hp_unit = max_HP / building_time;
		current_HP += hp_unit;
	}

}


void Building::HandleResourceProduction()
{
	if (timer.ReadSec() >= 3 && !App->map->WalkabilityArea(position.x - (additional_size.x * App->map->data.tile_width / 2) + collider.w / 2, (position.y - (additional_size.x * App->map->data.tile_width / 2)) + collider.h / 2, additional_size.x, additional_size.y, false, true))
	{
		timer.Start();
		App->scene->wood += resource_production;
	}
}

void Building::CalculateResourceProduction()
{
	float production_modifier = App->entitycontroller->worker_wood_production * (1 - (float)((villagers_inside - 1) * 0.05f));
	resource_production = 3 * villagers_inside * production_modifier;
}

void Building::Draw(float dt)
{
	App->render->Blit(texture, position.x, position.y, current_sprite);
}