#include "UI_FarmWorkersManager.h"
#include "j1App.h"
#include "j1EntityController.h"
#include "UI_Image.h"
#include "UI_Text.h"
#include "UI_ProgressBar.h"
#include "Building.h"

FarmWorkersManager::FarmWorkersManager(Building* building): UI_element(1375, 830, PRODUCTIONDISPLAY, { 0,0,200,220 }, (j1Module*)App->uiscene)
{
	for (std::list<worker*>::iterator it_w = building->workers_inside.begin(); it_w != building->workers_inside.end(); it_w++)
	{
		workerIcon* icon = new workerIcon((*it_w));
		icon->icon->parent = this;
		icons.push_back(icon);
	}
}


FarmWorkersManager::~FarmWorkersManager()
{
}

void FarmWorkersManager::BlitElement()
{
	for (std::vector<workerIcon*>::iterator it_i = icons.begin(); it_i != icons.end(); it_i++)
	{
		(*it_i)->icon->BlitElement();
	}
}

workerIcon::workerIcon(worker* worker)
{
	Worker = worker;
	state = UNASSIGNED;
	icon = new Image(App->gui->icon_atlas, 0, 0, { 9,7,72,60 }, App->uiscene);
	icon->setBorder(true);
}
