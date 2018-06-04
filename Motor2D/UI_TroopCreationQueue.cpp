#include "UI_TroopCreationQueue.h"
#include "j1App.h"
#include "j1EntityController.h"
#include "UI_SelectionDisplay.h"
#include "UI_Image.h"
#include "UI_ProgressBar.h"
#include "Entity.h"
#include "Unit.h"
#include "j1Timer.h"
#include "UI_Text.h"
#include "j1Fonts.h"
#include "j1Render.h"

TroopCreationQueue::TroopCreationQueue(Building* building): UI_element(875,855,PRODUCTIONDISPLAY, {0,0,200,220}, (j1Module*)App->uiscene)
{
	this->building = building;
	text = new Text("Troops being created:", 800, 815, App->font->getFont(1), { 255,255,255,255 }, nullptr);
}

TroopCreationQueue::~TroopCreationQueue()
{
	icons.clear();
	RELEASE(text);
}

void TroopCreationQueue::BlitElement()
{
	if (building != nullptr && building->isSelected)
		text->BlitElement();

	int counterX = 0;
	int counterY = 0;
	iPoint icon_offset = ICONS_OFFSET;
	if (icons.size() > 0)
	{		
		icons.front()->progress->enterCurrentValue(icons.front()->timer.ReadSec());
	}

	int counter = 0;
	for (std::list<troopCreation*>::iterator it_i = icons.begin(); it_i != icons.end(); it_i++)
	{		
		if ((*it_i)->icon->image->state == CLICKED)
		{
			childs.remove((*it_i)->icon->image);
			RELEASE((*it_i));
			icons.erase(it_i);
			if (counter == 0)
			{
				building->timer.Start();
				it_i++;
				if (it_i != icons.end())
					(*it_i)->timer.Start();
				it_i--;
			}
			App->entitycontroller->RefundResources(building->unit_queue.at(counter));
			building->unit_queue.erase(building->unit_queue.begin()+counter);
			App->audio->PlayFx(SFX_MISCELLANEOUS_CANCEL_TROOP);

			continue;
		}
		if ((*it_i)->progress->progress == 1.0f)
		{
			childs.remove((*it_i)->icon->image);
			RELEASE((*it_i));
			icons.erase(it_i);
			it_i++;
			if (it_i != icons.end())
				(*it_i)->timer.Start();
			it_i--;
			continue;
		}

		if (building != nullptr && building->isSelected)
		{
			(*it_i)->icon->image->localPosition = { icon_offset.x*counterX, icon_offset.y*counterY };
			(*it_i)->icon->image->BlitElement();
			if ((*it_i)->icon->image->state == MOUSEOVER || (*it_i)->icon->image->state == CLICKED)
			{
				iPoint pos = (*it_i)->icon->image->calculateAbsolutePosition();
				SDL_Rect section = { pos.x, pos.y, (*it_i)->icon->image->section.w, (*it_i)->icon->image->section.h };
				App->render->DrawQuad(section, Translucid_Red, true, false, true);
			}
			(*it_i)->progress->localPosition = { icon_offset.x*counterX, icon_offset.y*counterY + 63 };
			(*it_i)->progress->BlitElement();
			counterY++;
			counter++;
			if (counter == 4)
				break;
			if (counterY == 2)
			{
				counterY = 0;
				counterX++;
			}
		}
	}
}

void TroopCreationQueue::pushTroop(Type type)
{
	troopCreation* display = new troopCreation();
	TroopIcon* icon = new TroopIcon(App->entitycontroller->getUnitFromDB(type), 0, 0);
	icon->image->parent = this;
	App->gui->createPopUpInfo(icon->image, "Click to remove squad");
	icon->image->callback = App->uiscene;
	display->icon = icon;

	Entity* entity = (Entity*)App->entitycontroller->getUnitFromDB(type);
	ProgressBar* bar = new ProgressBar(0, 0, App->gui->atlas, { 285, 399, 69, 11 }, { 285, 421, 69, 11 }, { 0,0,0,0 }, entity->cost.creation_time, nullptr);
	bar->progress = 0.0f;
	bar->parent = this;
	display->progress = bar;

	display->timer.Start();
	icons.push_back(display);
	childs.push_back(display->icon->image);
}

UI_element* TroopCreationQueue::getMouseHoveringElement()
{
	UI_element* ret = this;

	for (std::list<troopCreation*>::iterator it_i = icons.begin(); it_i != icons.end(); it_i++)
	{
		if (App->gui->checkMouseHovering((*it_i)->icon->image))
		{
			ret = (*it_i)->icon->image;
			break;
		}
	}

	return ret;
}
