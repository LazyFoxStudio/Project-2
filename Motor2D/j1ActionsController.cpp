#include "j1ActionsController.h"
#include "j1App.h"
#include "j1Input.h"
#include "Command.h"
#include "j1Render.h"
#include "j1EntityController.h"

bool j1ActionsController::Update(float dt)
{
	if (doingAction)
	{
		//Blit mouse effect
		int x, y;
		App->input->GetMousePosition(x, y);
		Color yellow(255, 255, 0, 150);
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
	}

	return true;
}

void j1ActionsController::activateAction(actionType type)
{
	doingAction = true;

	action_type = type;
}
