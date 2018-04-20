#include "Building.h"
#include "j1Render.h"
#include "j1Scene.h"
#include "j1UIScene.h"
#include "j1EntityController.h"
#include "j1Input.h"
#include "j1Scene.h"
#include "j1Audio.h"
#include "UI_Chrono.h"

Building::Building(iPoint pos, Building& building)
{
	texture				= building.texture;
	type				= building.type;
	cost				= building.cost;

	current_HP = max_HP	= building.max_HP;
	defense				= building.defense;

	collider.x = position.x = pos.x;
	collider.y = position.y	= pos.y;

	size.x = building.size.x;
	size.y = building.size.y;

	collider.w = building.collider.w;
	collider.h = building.collider.h;

	additional_size.x = building.additional_size.x;
	additional_size.y = building.additional_size.y;

	for (int i = 0; i < 9; i++)
		available_actions = building.available_actions;

	sprites = building.sprites;

	if (type == TOWN_HALL)
	{
		ex_state = OPERATIVE;
		current_sprite = &sprites[1];
	}
	else current_sprite = &sprites[CREATION_STARTED];

	timer.Start();
}


bool Building::Update(float dt)
{
	//minimap_
	if (App->uiscene->minimap) 
	{
		if(ex_state != DESTROYED)	App->uiscene->minimap->Addpoint({ (int)position.x,(int)position.y,100,100 }, Green);
		else						App->uiscene->minimap->Addpoint({ (int)position.x,(int)position.y,100,100 }, Grey);
	}

	if(current_HP <= 0 && ex_state != DESTROYED)   Destroy();

	switch (ex_state)
	{
	case BEING_BUILT:
		HandleConstruction();
		if (timer.ReadSec() >= cost.creation_time / 2 && current_sprite == &sprites[2])
			current_sprite = &sprites[HALF_CREATED];
		break;
	case OPERATIVE:

		if (type == LUMBER_MILL)
			HandleResourceProduction();
		break;
	case DESTROYED:
		if (timer.ReadSec() > DEATH_TIME)
			App->entitycontroller->entities_to_destroy.push_back(this);

		break;
	}

	return true;
}

void Building::Destroy()
{
	ex_state = DESTROYED;
	App->entitycontroller->selected_entities.remove(this);
	current_sprite = &sprites[RUIN];

	switch (type)
	{
	case FARM:
		if (App->scene->inactive_workers >= 5)
		{
			App->scene->inactive_workers -= 5;
			App->scene->workers -= 5;
		}
		break;
	case LUMBER_MILL:
		App->entitycontroller->GetTotalIncome();
		break;
	case TOWN_HALL:
		App->audio->PlayMusic(DEFEAT_THEME);
		current_sprite = &sprites[4];
		App->gui->Chronos->counter.PauseTimer();
		App->scene->toRestart = true;
		App->scene->Restart_timer.Start();
		App->uiscene->toggleMenu(true, GAMEOVER_MENU);
		break;
	}

	timer.Start();
}


void Building::HandleConstruction()
{
	int current_time = timer.ReadSec();
	if (current_time >= cost.creation_time)
	{
		current_HP = max_HP;
		App->scene->inactive_workers += 1;
		timer.Start();
		if (type == FARM)
		{
			App->scene->workers += 5;
			App->scene->inactive_workers += 5;
		}
		ex_state = OPERATIVE;
		current_sprite = &sprites[COMPLETE];
	}
	else if (current_time > last_frame_time)
	{
		last_frame_time = current_time;
		current_HP += (max_HP / cost.creation_time);
	}

}


void Building::HandleResourceProduction()
{
	if (timer.ReadSec() >= 3)
	{
		timer.Start();
		if(!App->map->WalkabilityArea(position.x - (additional_size.x * App->map->data.tile_width / 2) + collider.w / 2, (position.y - (additional_size.x * App->map->data.tile_width / 2)) + collider.h / 2, additional_size.x, additional_size.y, false, true))
			App->scene->wood += resource_production;
	}
}

void Building::CalculateResourceProduction()
{
	float production_modifier = WOOD_PER_WORKER * (1 - (float)((villagers_inside - 1) * 0.05f));
	resource_production = 3 * villagers_inside * production_modifier;
}

void Building::Draw(float dt)
{
	App->render->Blit(texture, position.x, position.y, current_sprite);
}