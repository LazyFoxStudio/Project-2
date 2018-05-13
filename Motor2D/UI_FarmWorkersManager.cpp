#include "UI_FarmWorkersManager.h"
#include "j1App.h"
#include "j1EntityController.h"
#include "UI_Image.h"
#include "UI_Text.h"
#include "UI_ProgressBar.h"
#include "Building.h"
#include "j1Fonts.h"
#include "j1Render.h"

FarmWorkersManager::FarmWorkersManager(): UI_element(1400, 800, PRODUCTIONDISPLAY, { 0,0,200,220 }, (j1Module*)App->uiscene)
{
	int counterX = 0;
	int counterY = 0;
	for (int i = 0; i < MAX_VILLAGERS_FARM; i++)
	{
		workerIcon* icon = new workerIcon(0 + (85*counterX), 0 + (80*counterY));
		icon->icon->parent = this;
		icon->dark_icon->parent = this;
		icons.push_back(icon);
		counterX++;
		if (counterX == 3)
		{
			counterY++;
			counterX = 0;
		}
	}
}


FarmWorkersManager::~FarmWorkersManager()
{
}

void FarmWorkersManager::BlitElement()
{
	if (App->entitycontroller->selected_entities.size() > 0 && App->entitycontroller->selected_entities.front()->type == FARM)
	{
		Building* building = (Building*)App->entitycontroller->selected_entities.front();
		std::vector<workerIcon*>::iterator it_i = icons.begin();
		if (it_i != icons.end())
		{
			for (std::list<worker*>::reverse_iterator it_w = building->workers_inside.rbegin(); it_w != building->workers_inside.rend(); it_w++)
			{
				if ((*it_w)->working_at != nullptr)
					(*it_i)->Draw(WORKING);
				else
					(*it_i)->Draw(UNASSIGNED);

				it_i++;
			}
			if (it_i != icons.end())
			{
				(*it_i)->Draw(GENERATING);
				it_i++;
			}
			while (it_i != icons.end())
			{
				(*it_i)->Draw(WAITING);
				it_i++;
			}
		}
	}
}

workerIcon::workerIcon(int x, int y)
{
	//Create the icon image
	icon = new Image(App->gui->atlas, x, y, { 750,271,66,54 }, App->uiscene);
	icon->setBorder(true);

	//Create the dark icon image
	dark_icon = new Image(App->gui->atlas, x, y, { 823,271,66,54 }, App->uiscene);
	dark_icon->setBorder(true);

	//Create state texts
	unassigned = new Text("Unassigned", 0, 0, App->font->getFont(5), { 255,255,0,255 }, nullptr);
	unassigned->active = false;
	icon->appendChild(unassigned, true);
	unassigned->localPosition.y -= 35;
	working = new Text("Working", 0, 0, App->font->getFont(5), { 50,255,50,255 }, nullptr);
	working->active = false;
	icon->appendChild(working, true);
	working->localPosition.y -= 35;
	generating = new Text("Generating", 0, 0, App->font->getFont(5), { 255,255,255,255 }, nullptr);
	generating->active = false;
	dark_icon->appendChild(generating, true);
	generating->localPosition.y -= 35;
	waiting = new Text("Waiting", 0, 0, App->font->getFont(5), { 255,50,0,255 }, nullptr);
	waiting->active = false;
	dark_icon->appendChild(waiting, true);
	waiting->localPosition.y -= 35;

	//Create progress bar
	progress = new ProgressBar(0, 0, App->gui->atlas, { 285, 399, 69, 11 }, { 285, 421, 69, 11 }, { 0,0,0,0 }, FARM_WORKER_PRODUCTION_SECONDS, nullptr);
	dark_icon->appendChild(progress, true);
	progress->progress = 0.0f;
	progress->localPosition.y += 22;
}

void workerIcon::Draw(worker_state state)
{
	Building* building = (Building*)App->entitycontroller->selected_entities.front();
	SDL_Rect section = { icon->calculateAbsolutePosition().x, icon->calculateAbsolutePosition().y, icon->section.w, icon->section.h };
	Color color = { 200,200,200,150 };
	unassigned->active = false;
	working->active = false;
	generating->active = false;
	waiting->active = false;
	switch (state)
	{
	case UNASSIGNED:
		unassigned->active = true;
		icon->BlitElement();
		break;
	case WORKING:
		working->active = true;
		icon->BlitElement();
		break;
	case GENERATING:
		progress->enterCurrentValue(building->timer.ReadSec());
		generating->active = true;
		dark_icon->BlitElement();
		break;
	case WAITING:
		progress->enterCurrentValue(0);
		waiting->active = true;
		dark_icon->BlitElement();
		break;
	}
}
