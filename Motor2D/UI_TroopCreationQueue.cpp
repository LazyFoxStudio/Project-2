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

TroopCreationQueue::TroopCreationQueue(Building* building): UI_element(875,810,PRODUCTIONDISPLAY, {0,0,200,220}, (j1Module*)App->uiscene)
{
	this->building = building;
	text = new Text("Troops being created:", 660, 810, App->font->getFont(1), { 0,0,0,255 }, nullptr);
}

TroopCreationQueue::~TroopCreationQueue()
{
	icons.clear();
	RELEASE(text);
}

void TroopCreationQueue::BlitElement()
{
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
			RELEASE((*it_i));
			icons.erase(it_i);
			building->unit_queue.erase(building->unit_queue.begin()+counter);
			continue;
		}
		if ((*it_i)->progress->progress == 1.0f)
		{
			RELEASE((*it_i));
			icons.erase(it_i);
			it_i++;
			if (it_i != icons.end())
				(*it_i)->timer.Start();
			it_i--;
			continue;
		}
		(*it_i)->icon->image->localPosition = { icon_offset.x*counterX, icon_offset.y*counterY };
		(*it_i)->icon->image->BlitElement();
		(*it_i)->progress->localPosition = { icon_offset.x*counterX, icon_offset.y*counterY +63};
		(*it_i)->progress->BlitElement();
		counterY++;
		counter++;
		if (counter == 6)
			break;
		if (counterY == 3)
		{
			counterY = 0;
			counterX++;
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
