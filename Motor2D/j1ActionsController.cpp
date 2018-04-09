#include "j1ActionsController.h"
#include "j1App.h"
#include "j1Input.h"
#include "Command.h"
#include "j1Render.h"
#include "j1EntityController.h"
#include "j1Scene.h"

bool j1ActionsController::Update(float dt)
{
	if (doingAction)
	{
		Color yellow(255, 255, 0, 150);
		switch (action_type)
		{
		case MOVE:
			//Blit mouse effect
			int x, y;
			App->input->GetMousePosition(x, y);
			App->render->DrawQuad({ x, y, 10, 10 }, yellow, true, false);
			if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN)
			{
				App->entitycontroller->commandControl();
				action_assigned = true;
			}
			else if (action_assigned && App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP)
			{
				doingAction = false;
				action_assigned = false;
			}
			if (App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
			{
				doingAction = false;
			}
			break;
		case BUILD_BARRACKS:
			if (doingAction && !App->entitycontroller->building && App->scene->workerAvalible() && App->entitycontroller->CheckCostBuiding(BARRACKS))
			{
				App->entitycontroller->structure_beingbuilt = BARRACKS;
				App->entitycontroller->building = true;
				App->scene->inactive_workers -= 1;
			}
			else if (!App->scene->workerAvalible() || !App->entitycontroller->CheckCostBuiding(BARRACKS))
			{
				doingAction = false;
			}
			break;
		case BUILD_LUMBER_MILL:
			if (doingAction && !App->entitycontroller->building && App->scene->workerAvalible() && App->entitycontroller->CheckCostBuiding(LUMBER_MILL))
			{
				App->entitycontroller->structure_beingbuilt = LUMBER_MILL;
				App->entitycontroller->building = true;
				App->scene->inactive_workers -= 1;
			}
			else if (!App->scene->workerAvalible() || !App->entitycontroller->CheckCostBuiding(LUMBER_MILL))
			{
				doingAction = false;
			}
			break;
		case BUILD_FARM:
			if (doingAction && !App->entitycontroller->building && App->scene->workerAvalible() && App->entitycontroller->CheckCostBuiding(FARM))
			{
				App->entitycontroller->structure_beingbuilt = FARM;
				App->entitycontroller->building = true;
				App->scene->inactive_workers -= 1;
			}
			else if (!App->scene->workerAvalible() || !App->entitycontroller->CheckCostBuiding(FARM))
			{
				doingAction = false;
			}
			break;
		case UNASSIGN_WORKER:
			App->entitycontroller->HandleWorkerAssignment(false, (Building*)*App->entitycontroller->selected_entities.begin());
			doingAction = false;
			break;
		case ASSIGN_WORKER:
			App->entitycontroller->HandleWorkerAssignment(true, (Building*)*App->entitycontroller->selected_entities.begin());
			doingAction = false;
			break;
		}
		
	}

	return true;
}

void j1ActionsController::activateAction(actionType type)
{
	doingAction = true;

	action_type = type;
}