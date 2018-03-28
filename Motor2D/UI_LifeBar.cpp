#include "UI_LifeBar.h"
#include "Entity.h"
#include "Unit.h"
#include "Building.h"
#include "UI_ProgressBar.h"

LifeBar::LifeBar(Entity * entity, SDL_Texture* texture): UI_element(0,0,PROGRESSBAR, {0,0,109,14}, nullptr)
{
	SDL_Rect empty = { 0,0,0,0 };
	SDL_Rect full = { 0,0,0,0 };

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
	
	if (bar->progress < 1.1f)
	{
		bar->localPosition = { (int)entity->position.x+offset.x, (int)entity->position.y+offset.y };

		bar->BlitElement(true);
	}
}
