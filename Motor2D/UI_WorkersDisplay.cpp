#include "UI_WorkersDisplay.h"
#include "j1App.h"
#include "j1Gui.h"
#include "Building.h"
#include "UI_Button.h"
#include "UI_Image.h"
#include "UI_Text.h"
#include "j1EntityController.h"
#include "j1Fonts.h"

WorkersDisplay::WorkersDisplay(SDL_Rect icon, Button* increase, Button* decrease, Building* building) : UI_element(0, 0, element_type::WORKERSDISPLAY, { 0,0,100,55 }, (j1Module*)App->uiscene),
increase(increase),
decrease(decrease),
building(building)
{
	this->icon = new Image(App->gui->atlas, 10, 25, icon, nullptr);
	this->icon->use_camera = true;
	workers = new Text("", 45, 22, App->font->fonts.front(), { 255,255,255,255 }, nullptr);
	workers->use_camera = true;
	increase->use_camera = true;
	decrease->use_camera = true;
	use_camera = true;
}

WorkersDisplay::WorkersDisplay(WorkersDisplay* copy, Building* building): UI_element(0, 0, element_type::WORKERSDISPLAY, { 0,0,100,55 }, (j1Module*)App->uiscene)
{
	icon = new Image();
	*icon = *copy->icon;
	icon->parent = this;
	workers = new Text();
	*workers = *copy->workers;
	workers->parent = this;
	increase = new Button();
	*increase = *copy->increase;
	increase->parent = this;
	decrease = new Button();
	*decrease = *copy->decrease;
	decrease->parent = this;
	this->building = building;
	use_camera = true;
}

WorkersDisplay::~WorkersDisplay()
{
	RELEASE(icon);
	RELEASE(increase);
	RELEASE(decrease);
}

void WorkersDisplay::BlitElement()
{
	if (building != nullptr && building->ex_state == OPERATIVE && (building->type == LUMBER_MILL || building->type == MINE))
	{
		//Hardcoded
		icon->localPosition = { (int)(10 * App->gui->w_stretch), (int)(25 * App->gui->h_stretch) };
		workers->localPosition = { (int)(45 * App->gui->w_stretch),(int)(22 * App->gui->h_stretch) };
		increase->localPosition = { (int)(60 * App->gui->w_stretch),(int)(0 * App->gui->h_stretch) };
		decrease->localPosition = { (int)(20 * App->gui->w_stretch), (int)(0 * App->gui->h_stretch) };
		// ----------------------
		iPoint offset = WORKERS_DISPLAY_OFFSET;
		localPosition = { (int)building->position.x + (int)(offset.x*App->gui->w_stretch), (int)building->position.y + (int)(offset.y*App->gui->h_stretch) };
		icon->BlitElement();
		std::string text = std::to_string(building->workers_inside.size()) + '/' + std::to_string(MAX_VILLAGERS_LUMBERMILL);
		workers->setText(text);
		workers->BlitElement();
		if (state == element_state::MOUSEOVER || state == element_state::CLICKED || building->isSelected)
		{
			increase->BlitElement();
			decrease->BlitElement();
		}
		else
		{
			SDL_SetTextureAlphaMod(increase->texture, 100);
			increase->BlitElement();
			SDL_SetTextureAlphaMod(decrease->texture, 100);
			decrease->BlitElement();
			SDL_SetTextureAlphaMod(increase->texture, 255);
			SDL_SetTextureAlphaMod(decrease->texture, 255);
		}
	}
}

UI_element* WorkersDisplay::getMouseHoveringElement()
{
	UI_element* ret = this;

	if (state == element_state::MOUSEOVER || state == element_state::CLICKED)
	{
		if (App->gui->checkMouseHovering(increase))
			ret = increase;
		else if (App->gui->checkMouseHovering(decrease))
			ret = decrease;
	}

	return ret;
}
