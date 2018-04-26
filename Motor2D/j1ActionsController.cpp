#include "j1ActionsController.h"
#include "j1App.h"
#include "j1Input.h"
#include "Command.h"
#include "j1Render.h"
#include "j1EntityController.h"
#include "j1Scene.h"
#include "j1Gui.h"
#include "UI_WarningMessages.h"

bool j1ActionsController::Update(float dt)
{
	BROFILER_CATEGORY("Actions Update", Profiler::Color::AliceBlue);
	if (doingAction)
	{
		switch (action_type)
		{
		case MOVE:
			//Blit mouse effect
			int x, y;
			App->input->GetMousePosition(x, y);
			App->render->DrawQuad({ x, y, 10, 10 }, Translucid_Yellow, true, false);
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
			if (doingAction)
				App->entitycontroller->to_build_type = BARRACKS;
			
			break;
		case BUILD_LUMBER_MILL:
			if (doingAction)
				App->entitycontroller->to_build_type = LUMBER_MILL;
			
			break;
		case BUILD_FARM:
			if (doingAction)
				App->entitycontroller->to_build_type = FARM;
			
			break;
		case UNASSIGN_WORKER:
			if (!App->entitycontroller->selected_entities.empty())
			{
				if (((Building*)*App->entitycontroller->selected_entities.begin())->ex_state != BEING_BUILT)
					App->entitycontroller->HandleWorkerAssignment(false, (Building*)*App->entitycontroller->selected_entities.begin());
			}
			doingAction = false;
			break;
		case ASSIGN_WORKER:
			if (!App->entitycontroller->selected_entities.empty())
			{
				if (((Building*)*App->entitycontroller->selected_entities.begin())->ex_state != BEING_BUILT)
					App->entitycontroller->HandleWorkerAssignment(true, (Building*)*App->entitycontroller->selected_entities.begin());
			}
			doingAction = false;
			break;
		case CREATE_FOOTMAN:
			if (!App->entitycontroller->selected_entities.empty())
			{
				if (App->entitycontroller->CheckCost(FOOTMAN) && ((Building*)*App->entitycontroller->selected_entities.begin())->ex_state != BEING_BUILT)
					App->entitycontroller->AddSquad(FOOTMAN, newSquadPos);
			}
			doingAction = false;
			break;
		case CREATE_ARCHER:
			if (!App->entitycontroller->selected_entities.empty())
			{
				if (App->entitycontroller->CheckCost(ARCHER) && ((Building*)*App->entitycontroller->selected_entities.begin())->ex_state != BEING_BUILT)
					App->entitycontroller->AddSquad(ARCHER, newSquadPos);
			}
			doingAction = false;
			break;
		case USE_ABILITY1:
			App->entitycontroller->hero->current_skill != 1 ? App->entitycontroller->hero->current_skill = 1 : App->entitycontroller->hero->current_skill = 0;
			doingAction = false;
			break;
		case USE_ABILITY2:
			App->entitycontroller->hero->current_skill != 2 ? App->entitycontroller->hero->current_skill = 2 : App->entitycontroller->hero->current_skill = 0;
			doingAction = false;
			break;
		case USE_ABILITY3:
			App->entitycontroller->hero->current_skill != 3 ? App->entitycontroller->hero->current_skill = 3 : App->entitycontroller->hero->current_skill = 0;
			doingAction = false;
			break;
		case REPAIR_BUILDING:
			if (!App->entitycontroller->selected_entities.empty())
			{
				if (((Building*)*App->entitycontroller->selected_entities.begin())->ex_state == OPERATIVE)
				{
					((Building*)*App->entitycontroller->selected_entities.begin())->RepairBuilding();
				}
			}
			doingAction = false;
			break;
		}
		
	}
	if (doingAction == true)
		doingAction_lastFrame = true;
	else
	{
		if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_IDLE)
			doingAction_lastFrame = false;
	}

	return true;
}

void j1ActionsController::activateAction(actionType type)
{
	if (doingAction || type == NO_ACTION)
	{
		App->gui->warningMessages->hideMessage(NO_WORKERS);
		App->gui->warningMessages->hideMessage(NO_RESOURCES);
		App->gui->warningMessages->hideMessage(NO_TREES);

		if (type == NO_ACTION)
		{
			doingAction = false;
			doingAction_lastFrame = false;
		}
	}
	else
		doingAction = true;

	action_type = type;
}