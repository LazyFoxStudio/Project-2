#include "UI_FarmWorkersManager.h"
#include "j1EntityController.h"
#include "UI_Image.h"
#include "UI_Text.h"
#include "UI_ProgressBar.h"

FarmWorkersManager::FarmWorkersManager()
{
}


FarmWorkersManager::~FarmWorkersManager()
{
}

workerIcon::workerIcon(worker* worker)
{
	Worker = worker;
	state = UNASSIGNED;
	icon = new Image(App->gui->icon_atlas, 0, 0, { 9,7,72,60 }, App->uiscene);
	icon->setBorder(true);
}
