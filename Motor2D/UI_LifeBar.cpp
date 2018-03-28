#include "UI_LifeBar.h"
#include "Entity.h"
#include "Unit.h"
#include "Building.h"
#include "UI_ProgressBar.h"

LifeBar::LifeBar(Entity * entity, SDL_Texture* texture) : UI_element(0, 0, PROGRESSBAR, { 0,0,109,14 }, nullptr)
{
	switch (entity->entity_type)
	{
	case UNIT:
		if (((Unit*)entity)->IsEnemy())
			bar = new ProgressBar(0, 0, texture, App->gui->GetLifeBarRect("R_empty"), App->gui->GetLifeBarRect("R_full"), { 0,0,0,0 }, ((Unit*)entity)->max_HP, callback);
		else
			bar = new ProgressBar(0, 0, texture, App->gui->GetLifeBarRect("G_empty"), App->gui->GetLifeBarRect("G_full"), { 0,0,0,0 }, ((Unit*)entity)->max_HP, callback);
		offset.x = entity->collider.w / 2 - bar->section.w / 2;
		break;
	case BUILDING:
		bar = new ProgressBar(0, 0, texture, App->gui->GetLifeBarRect("Y_empty"), App->gui->GetLifeBarRect("Y_full"), { 0,0,0,0 }, ((Building*)entity)->max_HP, callback);
		offset.x = ((Building*)entity)->section->w / 2 - bar->section.w / 2;
		break;
	}
	offset.y = -(5 + bar->section.h);

	this->entity = entity;
	this->inMenu = false;
}

LifeBar::LifeBar(Entity * entity, SDL_Texture * texture, int x, int y)
{
	float life = 1.0f;
	int max_value = 100;
	switch (entity->entity_type)
	{
	case UNIT:
		max_value = ((Unit*)entity)->max_HP;
		life = ((Unit*)entity)->current_HP / max_value;
		break;
	case BUILDING:
		max_value = ((Building*)entity)->max_HP;
		life = ((Building*)entity)->current_HP / max_value;
		break;
	}
	if (life >= 0.5)
		bar = new ProgressBar(x, y, texture, App->gui->GetLifeBarRect("mG_empty"), App->gui->GetLifeBarRect("mG_full"), { 0,0,0,0 }, max_value, callback);
	else if (life >= 0.2)
		bar = new ProgressBar(x, y, texture, App->gui->GetLifeBarRect("mR_empty"), App->gui->GetLifeBarRect("mR_full"), { 0,0,0,0 }, max_value, callback);
	else
		bar = new ProgressBar(x, y, texture, App->gui->GetLifeBarRect("mY_empty"), App->gui->GetLifeBarRect("mY_full"), { 0,0,0,0 }, max_value, callback);

	this->entity = entity;
	this->inMenu = true;
}

void LifeBar::BlitElement(bool use_camera)
{
	switch (entity->entity_type)
	{
	case UNIT:
		bar->enterCurrentValue(((Unit*)entity)->current_HP);
		break;
	case BUILDING:
		bar->enterCurrentValue(((Building*)entity)->current_HP);
		break;
	}
	if (inMenu)
	{
		if (bar->progress >= 0.5)
		{
			bar->section = App->gui->GetLifeBarRect("mG_empty");
			bar->full = App->gui->GetLifeBarRect("mG_full");
		}
		else if (bar->progress >= 0.2)
		{
			bar->section = App->gui->GetLifeBarRect("mY_empty");
			bar->full = App->gui->GetLifeBarRect("mY_full");
		}
		else
		{
			bar->section = App->gui->GetLifeBarRect("mR_empty");
			bar->full = App->gui->GetLifeBarRect("mR_full");
		}

		bar->BlitElement(false);
	}
	else if (bar->progress < 1.1f)
	{
		bar->localPosition = { (int)entity->position.x+offset.x, (int)entity->position.y+offset.y };
	
		bar->BlitElement(true);
	}
}
