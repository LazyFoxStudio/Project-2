#include "j1ActionsController.h"
#include "j1App.h"
#include "j1Input.h"
#include "Command.h"
#include "j1Render.h"
#include "j1EntityController.h"
#include "j1Scene.h"
#include "j1Gui.h"
#include "UI_WarningMessages.h"
#include "UI_WorkersDisplay.h"
#include "UI_NextWaveWindow.h"

bool j1ActionsController::Update(float dt)
{
	BROFILER_CATEGORY("Actions Update", Profiler::Color::AliceBlue);
	if (doingAction)
	{
		Hero* hero = nullptr;
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
		case BUILD_MINE:
			if (doingAction)
				App->entitycontroller->to_build_type = MINE;

			break;
		case BUILD_TURRET:
			if (doingAction)
				App->entitycontroller->to_build_type = TURRET;

			break;
		case BUILD_GNOME_HUT:
			if (doingAction)
				App->entitycontroller->to_build_type = GNOME_HUT;

			break;
		case BUILD_CHURCH:
			if (doingAction)
				App->entitycontroller->to_build_type = CHURCH;

			break;
		case BUILD_BLACKSMITH:
			if (doingAction)
				App->entitycontroller->to_build_type = BLACKSMITH;

			break;
		case UNASSIGN_WORKER:
			if (!App->entitycontroller->selected_entities.empty() && App->entitycontroller->selected_entities.front()->type == LUMBER_MILL)
			{
				if (((Building*)*App->entitycontroller->selected_entities.begin())->ex_state != BEING_BUILT)
					App->entitycontroller->HandleWorkerAssignment(false, (Building*)*App->entitycontroller->selected_entities.begin());
			}
			else
			{
				App->entitycontroller->HandleWorkerAssignment(false, ((WorkersDisplay*)App->gui->current_hovering_element->parent)->building);
			}
			doingAction = false;
			break;
		case ASSIGN_WORKER:
			if (!App->entitycontroller->selected_entities.empty() && App->entitycontroller->selected_entities.front()->type == LUMBER_MILL)
			{
				if (((Building*)*App->entitycontroller->selected_entities.begin())->ex_state != BEING_BUILT)
					App->entitycontroller->HandleWorkerAssignment(true, (Building*)*App->entitycontroller->selected_entities.begin());
			}
			else
			{
				App->entitycontroller->HandleWorkerAssignment(true, ((WorkersDisplay*)App->gui->current_hovering_element->parent)->building);
			}
			doingAction = false;
			break;
		case CREATE_FOOTMAN:
			if (!App->entitycontroller->selected_entities.empty())
			{
				if (App->entitycontroller->CheckCost(FOOTMAN) && ((Building*)*App->entitycontroller->selected_entities.begin())->ex_state != BEING_BUILT)
					((Building*)*App->entitycontroller->selected_entities.begin())->AddUnitToQueue(FOOTMAN);
			}
			doingAction = false;
			break;
		case CREATE_ARCHER:
			if (!App->entitycontroller->selected_entities.empty())
			{
				if (App->entitycontroller->CheckCost(ARCHER) && ((Building*)*App->entitycontroller->selected_entities.begin())->ex_state != BEING_BUILT)
					((Building*)*App->entitycontroller->selected_entities.begin())->AddUnitToQueue(ARCHER);
			}
			doingAction = false;
			break;
		case CREATE_KNIGHT:
			if (!App->entitycontroller->selected_entities.empty())
			{
				if (App->entitycontroller->CheckCost(KNIGHT) && ((Building*)*App->entitycontroller->selected_entities.begin())->ex_state != BEING_BUILT)
					((Building*)*App->entitycontroller->selected_entities.begin())->AddUnitToQueue(KNIGHT);
			}
			doingAction = false;
			break;
		case CREATE_GRYPHON:
			if (!App->entitycontroller->selected_entities.empty())
			{
				if (App->entitycontroller->CheckCost(GRYPHON) && ((Building*)*App->entitycontroller->selected_entities.begin())->ex_state != BEING_BUILT)
					((Building*)*App->entitycontroller->selected_entities.begin())->AddUnitToQueue(GRYPHON);
			}
			doingAction = false;
			break;
		/*case CREATE_BALLISTA:
			if (!App->entitycontroller->selected_entities.empty())
			{
			if (App->entitycontroller->CheckCost(GRYPHON) && ((Building*)*App->entitycontroller->selected_entities.begin())->ex_state != BEING_BUILT)
			App->entitycontroller->AddSquad(GRYPHON, newSquadPos);
			}
			doingAction = false;
			break;*/
		/*case CREATE_FLYING_MACHINE:
			if (!App->entitycontroller->selected_entities.empty())
			{
			if (App->entitycontroller->CheckCost(GRYPHON) && ((Building*)*App->entitycontroller->selected_entities.begin())->ex_state != BEING_BUILT)
			App->entitycontroller->AddSquad(GRYPHON, newSquadPos);
			}
			doingAction = false;
			break;*/
		case USE_ABILITY1:
			if (hero = (Hero*)App->entitycontroller->getEntitybyID(App->entitycontroller->hero_UID))
			{
				hero->current_skill != 1 ? hero->current_skill = 1 : hero->current_skill = 0;
				doingAction = false;
			}
			break;
		case USE_ABILITY2:
			if (hero = (Hero*)App->entitycontroller->getEntitybyID(App->entitycontroller->hero_UID))
			{
				hero->current_skill != 2 ? hero->current_skill = 2 : hero->current_skill = 0;
				doingAction = false;
			}
			break;
		case USE_ABILITY3:
			if (hero = (Hero*)App->entitycontroller->getEntitybyID(App->entitycontroller->hero_UID))
			{
				hero->current_skill != 3 ? hero->current_skill = 3 : hero->current_skill = 0;
				doingAction = false;
			}
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
		case DEMOLISH_BUILDING:
			if (!App->entitycontroller->selected_entities.empty())
			{
				if (((Building*)*App->entitycontroller->selected_entities.begin())->ex_state == OPERATIVE)
				{
					((Building*)*App->entitycontroller->selected_entities.begin())->DemolishBuilding();
				}
			}
			doingAction = false;
			break;
		case TOGGLE_NEXTWAVE:
			App->gui->nextWaveWindow->toggle();
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