#include "UI_LifeBar.h"
#include "Entity.h"
#include "Unit.h"
#include "Hero.h"
#include "Building.h"
#include "UI_ProgressBar.h"
#include "UI_Text.h"
#include "j1Fonts.h"

LifeBar::LifeBar(Entity * entity, SDL_Texture* texture) : UI_element(0, 0, PROGRESSBAR, { 0,0,109,14 }, nullptr)
{
	switch (entity->entity_type)
	{
	case UNIT:
		if (((Unit*)entity)->IsEnemy())
			bar = new ProgressBar(0, 0, texture, App->gui->GetLifeBarRect("R_empty"), App->gui->GetLifeBarRect("R_full"), { 0,0,0,0 }, ((Unit*)entity)->max_HP, callback);
		else
			bar = new ProgressBar(0, 0, texture, App->gui->GetLifeBarRect("G_empty"), App->gui->GetLifeBarRect("G_full"), { 0,0,0,0 }, ((Unit*)entity)->max_HP, callback);
		offset.x = -(bar->section.w / 2);
		break;
	case BUILDING:
		bar = new ProgressBar(0, 0, texture, App->gui->GetLifeBarRect("Y_empty"), App->gui->GetLifeBarRect("Y_full"), { 0,0,0,0 }, ((Building*)entity)->max_HP, callback);
		offset.x = ((Building*)entity)->sprites[0].w / 2 - bar->section.w / 2;
		break;
	/*case HERO:
			bar = new ProgressBar(0, 0, texture, App->gui->GetLifeBarRect("G_empty"), App->gui->GetLifeBarRect("G_full"), { 0,0,0,0 }, ((Hero*)entity)->max_HP, callback);
		offset.x = entity->collider.w / 2 - bar->section.w / 2;
		break;*/
	}
	offset.y = -(25 + bar->section.h);

	this->entity = entity;
	this->inMenu = false;
}

LifeBar::LifeBar(Entity * entity, SDL_Texture * texture, int x, int y)
{
	float progress = 1.0f;
	int life = 100;
	int max_value = 100;
	switch (entity->entity_type)
	{
	case UNIT:
		life = ((Unit*)entity)->current_HP;
		max_value = ((Unit*)entity)->max_HP;
		progress = life / max_value;
		break;
	case BUILDING:
		life = ((Building*)entity)->current_HP;
		max_value = ((Building*)entity)->max_HP;
		progress = life / max_value;
		break;
	/*case HERO:
		life = ((Hero*)entity)->current_HP;
		max_value = ((Hero*)entity)->max_HP;
		progress = life / max_value;
		break;*/
	}
	if (progress >= 0.5)
		bar = new ProgressBar(x, y, texture, App->gui->GetLifeBarRect("mG_empty"), App->gui->GetLifeBarRect("mG_full"), { 0,0,0,0 }, max_value, callback);
	else if (progress >= 0.2)
		bar = new ProgressBar(x, y, texture, App->gui->GetLifeBarRect("mR_empty"), App->gui->GetLifeBarRect("mR_full"), { 0,0,0,0 }, max_value, callback);
	else
		bar = new ProgressBar(x, y, texture, App->gui->GetLifeBarRect("mY_empty"), App->gui->GetLifeBarRect("mY_full"), { 0,0,0,0 }, max_value, callback);

	std::string text = std::to_string((int)life) + "/" + std::to_string((int)max_value);
	display = new Text(text, 0, 0, (*App->font->fonts.begin()), { 0,0,0,255 }, callback);
	display->setOutlined(true);
	display->setOutlineColor({ 255,255,255,255 });
	bar->appendChild(display, true);

	this->entity = entity;
	this->inMenu = true;
}

LifeBar::~LifeBar()
{
	//RELEASE(display); //HARDCODED
	RELEASE(bar);
}

void LifeBar::BlitElement(bool use_camera)
{
	//Update progressbar
	int life = 100;
	int max_life = 100;
	switch (entity->entity_type)
	{
	case UNIT:
		life = ((Unit*)entity)->current_HP;
		if (life < 0)
			life = 0;
		max_life = ((Unit*)entity)->max_HP;
		bar->enterCurrentValue(life);
		break;
	case BUILDING:
		life = ((Building*)entity)->current_HP;
		max_life = ((Building*)entity)->max_HP;
		if (life < 0)
			life = 0;
		bar->enterCurrentValue(life);
		break;
	/*case HERO:
		life = ((Hero*)entity)->current_HP;
		max_life = ((Hero*)entity)->max_HP;
		bar->enterCurrentValue(life);
		break;*/
	}
	//If in menu update bar color
	if (inMenu)
	{
		if (life / max_life != bar->progress)
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
				LOG("Character is dead");
				bar->section = App->gui->GetLifeBarRect("mR_empty");
				bar->full = App->gui->GetLifeBarRect("mR_full");
			}
		}
		std::string text = std::to_string((int)(bar->max_value*bar->progress)) + "/" + std::to_string((int)bar->max_value); //PERFORMANCE ISSUE? (creating string each frame)
		display->setText(text);

		bar->BlitElement(false);
	}
	else if (bar->progress < 1.0f) //If "in-game" but 100% do not draw
	{
		bar->localPosition = { (int)entity->position.x+offset.x, (int)entity->position.y+offset.y };
	
		bar->BlitElement(true);
	}

	UI_element::BlitElement(use_camera);
}
