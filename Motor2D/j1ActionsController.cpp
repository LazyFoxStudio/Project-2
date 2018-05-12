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
#include "j1WaveController.h"
#include "j1Tutorial.h"

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
			if (!App->entitycontroller->selected_entities.empty() &&
				(App->entitycontroller->selected_entities.front()->type == LUMBER_MILL|| App->entitycontroller->selected_entities.front()->type == MINE) &&
				App->gui->current_hovering_element != nullptr && (App->gui->current_hovering_element->parent == nullptr || App->gui->current_hovering_element->parent->element_type != WORKERSDISPLAY))
			{
				if (((Building*)*App->entitycontroller->selected_entities.begin())->ex_state != BEING_BUILT)
					App->entitycontroller->HandleWorkerAssignment(false, (Building*)*App->entitycontroller->selected_entities.begin());
			}
			else if (App->gui->current_hovering_element != nullptr && App->gui->current_hovering_element->parent != nullptr)
			{
				App->entitycontroller->HandleWorkerAssignment(false, ((WorkersDisplay*)App->gui->current_hovering_element->parent)->building);
			}
			doingAction = false;
			break;
		case ASSIGN_WORKER:
			if (!App->entitycontroller->selected_entities.empty() &&
				(App->entitycontroller->selected_entities.front()->type == LUMBER_MILL || App->entitycontroller->selected_entities.front()->type == MINE) &&
				App->gui->current_hovering_element != nullptr && (App->gui->current_hovering_element->parent == nullptr || App->gui->current_hovering_element->parent->element_type != WORKERSDISPLAY))
			{
				if (((Building*)*App->entitycontroller->selected_entities.begin())->ex_state != BEING_BUILT)
					App->entitycontroller->HandleWorkerAssignment(true, (Building*)*App->entitycontroller->selected_entities.begin());
			}
			else if (App->gui->current_hovering_element != nullptr && App->gui->current_hovering_element->parent != nullptr)
			{
				App->entitycontroller->HandleWorkerAssignment(true, ((WorkersDisplay*)App->gui->current_hovering_element->parent)->building);
			}
			doingAction = false;
			break;
		case CREATE_FOOTMAN:
			if (!App->entitycontroller->selected_entities.empty())
			{
				if (App->entitycontroller->CheckCost(FOOTMAN) && ((Building*)*App->entitycontroller->selected_entities.begin())->ex_state != BEING_BUILT)
				{
					((Building*)*App->entitycontroller->selected_entities.begin())->AddUnitToQueue(FOOTMAN);
				}
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
		case CREATE_BALLISTA:
			if (!App->entitycontroller->selected_entities.empty())
			{
				if (App->entitycontroller->CheckCost(BALLISTA) && ((Building*)*App->entitycontroller->selected_entities.begin())->ex_state != BEING_BUILT)
					((Building*)*App->entitycontroller->selected_entities.begin())->AddUnitToQueue(BALLISTA);
			}
			doingAction = false;
			break;
		case CREATE_FLYING_MACHINE:
			if (!App->entitycontroller->selected_entities.empty())
			{
				if (App->entitycontroller->CheckCost(FLYING_MACHINE) && ((Building*)*App->entitycontroller->selected_entities.begin())->ex_state != BEING_BUILT)
					((Building*)*App->entitycontroller->selected_entities.begin())->AddUnitToQueue(FLYING_MACHINE);
			}
			doingAction = false;
			break;
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
		case RESEARCH_MELEE_ATTACK:
			if (App->entitycontroller->ChechUpgradeCost(MELEE_ATTACK_UPGRADE))
			{
				App->entitycontroller->UpgradeUnits(MELEE_ATTACK_UPGRADE);
				App->entitycontroller->SpendUpgradeResources(MELEE_ATTACK_UPGRADE);
			}
		case TOGGLE_NEXTWAVE:
			App->gui->nextWaveWindow->toggle();
			doingAction = false;
			break;
		case NEW_GAME:		
			if (App->tutorial->active)
				App->tutorial->startTutorial();
			App->scene->active = true;
			App->entitycontroller->active = true;
			App->wavecontroller->active = true;
			App->uiscene->toggleMenu(false, START_MENU);
			App->uiscene->toggleMenu(true, INGAME_MENU);			
			App->scene->Start_game();			
			doingAction = false;
			App->pauseGame();
			break;
		case CROSS_MENU:
		case BACK_MENU:
			if (App->uiscene->getMenu(CHANGE_HOTKEYS_MENU)->active)
				App->uiscene->toggleMenu(false, CHANGE_HOTKEYS_MENU);
			else if (App->uiscene->getMenu(PAUSE_MENU)->active && !App->uiscene->getMenu(SETTINGS_MENU)->active)
			{
				App->uiscene->toggleMenu(false, PAUSE_MENU);
				App->resumeGame();
			}
			else
			{
				App->uiscene->toggleMenu(false, CREDITS_MENU);
				App->uiscene->toggleMenu(false, SETTINGS_MENU);
			}

			if (App->input->isReadingHotkey())
				App->input->stopReadingHotkey();
			
			doingAction = false;
			break;
		case CREDITS:
			App->uiscene->toggleMenu(true, CREDITS_MENU);
			doingAction = false;
			break;
		case SETTINGS:
			App->uiscene->toggleMenu(true, SETTINGS_MENU);
			doingAction = false;
			break;
		case EXIT:
			doingAction = false;
			return false;
			break;
		case CHANGE_HOTKEYS:
			App->uiscene->toggleMenu(true, CHANGE_HOTKEYS_MENU);
			doingAction = false;
			break;
		case PAUSE:
			App->uiscene->toggleMenu(true, PAUSE_MENU);
			App->pauseGame();
			doingAction = false;
			break;
		case START_SCENE:
			App->resumeGame();
			App->scene->active = false;
			App->entitycontroller->active = false;
			App->wavecontroller->active = false;
			App->uiscene->toggleMenu(true, START_MENU);
			App->uiscene->toggleMenu(false, INGAME_MENU);
			App->uiscene->toggleMenu(false, PAUSE_MENU);
			App->uiscene->toggleMenu(false, HERO_SELECTION_MENU);
			doingAction = false;
			App->scene->Close_game();
			break;
		case CHOOSE_MAGE:
			App->entitycontroller->addHero(iPoint(1950, 2100), HERO_1);
			App->uiscene->toggleMenu(false, HERO_SELECTION_MENU);
			App->resumeGame();
			doingAction = false;
			break;
		case CHOOSE_PALADIN:
			App->entitycontroller->addHero(iPoint(1950, 2100), HERO_2);
			App->uiscene->toggleMenu(false, HERO_SELECTION_MENU);
			App->resumeGame();
			doingAction = false;
			break;

		case CHANGE_KEY_1:
			App->input->readHotkey(0);
			doingAction = false;
			break;
		case CHANGE_KEY_2:
			App->input->readHotkey(1);
			doingAction = false;
			break;
		case CHANGE_KEY_3:
			App->input->readHotkey(2);
			doingAction = false;
			break;
		case CHANGE_KEY_4:
			App->input->readHotkey(3);
			doingAction = false;
			break;
		case CHANGE_KEY_5:
			App->input->readHotkey(4);
			doingAction = false;
			break;
		case CHANGE_KEY_6:
			App->input->readHotkey(5);
			doingAction = false;
			break;
		case CHANGE_KEY_7:
			App->input->readHotkey(6);
			doingAction = false;
			break;
		case CHANGE_KEY_8:
			App->input->readHotkey(7);
			doingAction = false;
			break;
		case CHANGE_KEY_9:
			App->input->readHotkey(8);
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
	{
		doingAction = true;
		if (type == BUILD_LUMBER_MILL)
			if (App->tutorial->doingTutorial)
				App->tutorial->taskCompleted(PICK_LUMBER_MILL);
	}

	action_type = type;
}