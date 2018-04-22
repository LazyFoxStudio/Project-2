#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Input.h"
#include "j1Textures.h"
#include "j1Audio.h"
#include "j1Render.h"
#include "j1Window.h"
#include "j1Map.h"
#include "j1Scene.h"
#include "j1UIScene.h"
#include "j1Sceneswitch.h"
#include "j1Pathfinding.h"
#include "j1EntityController.h"
#include "j1Gui.h"
#include "UI_Button.h"
#include "j1EntityController.h"
#include "j1WaveController.h"
#include "UI_CostDisplay.h"
#include "UI_Chrono.h"
#include "UI_NextWaveWindow.h"
#include "Building.h"
#include "Squad.h"
#include "j1ActionsController.h"
#include "UI_WarningMessages.h"

j1Scene::j1Scene() : j1Module() { name = "scene"; }

// Destructor
j1Scene::~j1Scene() {}


// Called before the first frame
bool j1Scene::Start()
{
	int w = -1, h = -1;
	uchar* data = nullptr;

	App->audio->PlayMusic(MAIN_THEME);
	App->map->Load_map("map1.tmx");

	if (App->map->CreateWalkabilityMap(w, h, &data))	
		App->pathfinding->SetMap(w, h, data);

	pugi::xml_document	Gui_config_file;
	pugi::xml_node		guiconfig;

	guiconfig = App->LoadFile(Gui_config_file, "Gui_config.xml");
	guiconfig = guiconfig.child("scene");

	App->render->camera.x = -1200;
	App->render->camera.y = -1600;

	restart_time = 10;
	wood = INIT_WOOD;
	gold = INIT_GOLD;
	//inactive_workers = workers = INIT_WORKERS;
	town_hall_lvl = INIT_TOWNHALL_LVL;

	return true;
}

// Called each loop iteration
bool j1Scene::Update(float dt)
{
	BROFILER_CATEGORY("Scene update", Profiler::Color::Chocolate);
	App->render->MouseCameraMovement(dt);
	App->map->Draw();

	//Music and SFX modifiers (temporal for Vertical Slice)
	if (App->input->GetKey(SDL_SCANCODE_P) == KEY_DOWN)
	{
		App->audio->ModifyMusicVolume(10);
	}
	if (App->input->GetKey(SDL_SCANCODE_O) == KEY_DOWN)
	{
		App->audio->ModifyMusicVolume(-10);
	}
	if (App->input->GetKey(SDL_SCANCODE_L) == KEY_DOWN)
	{
		App->audio->ModifySFXVolume(10);
	}
	if (App->input->GetKey(SDL_SCANCODE_K) == KEY_DOWN)
	{
		App->audio->ModifySFXVolume(-10);
	}


	workers_int = workers.size();
	GetTotalInactiveWorkers();

	//TEST SPAWNER--------------------------------------------------
	/*if (App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
	{
		int x, y;
		iPoint mouse_pos;
		App->input->GetMousePosition(x,y);
		mouse_pos=App->render->ScreenToWorld(x, y);
		App->entitycontroller->AddSquad(FOOTMAN, {(float)mouse_pos.x,(float)mouse_pos.y});
	}
	*/

	return true;
}

// Called each loop iteration
bool j1Scene::PostUpdate()
{
	if (App->input->GetKey(SDL_SCANCODE_F4) == KEY_DOWN )
		return false;

	if (toRestart && Restart_timer.ReadSec() >= restart_time) Restart_game();
	return true;
}

// Called before quitting
bool j1Scene::CleanUp()
{
	LOG("Freeing scene");

	return true;
}

bool j1Scene::Load(pugi::xml_node& data)
{
	return true;
}

bool j1Scene::Save(pugi::xml_node& data) const
{
	return true;
}

//bool j1Scene::workerAvalible(int num)
//{
//	bool ret = false;
//
//	if (inactive_workers == 0)
//	{
//		ret = false;
//	}
//	else if (inactive_workers >= num)
//	{
//		ret = true;
//	}
//
//	return ret;
//}

void j1Scene::Restart_game()
{

	//DELETING ENTITIES-------------------------------------------------------
	std::list<Entity*>::iterator it = App->entitycontroller->entities.begin();
	while (it != App->entitycontroller->entities.end() && !App->entitycontroller->entities.empty() && (*it) != nullptr)
	{
		App->entitycontroller->DeleteEntity(*it);
		it++;
	}

	for (std::list<Squad*>::iterator it = App->entitycontroller->squads.begin(); it != App->entitycontroller->squads.end() && !App->entitycontroller->squads.empty() && (*it) != nullptr; it++)
	{
		if ((*it)->units.empty())
		{/*
			if (*App->entitycontroller->squad_iterator == (*it))
				App->entitycontroller->squad_iterator = App->entitycontroller->all_squads.begin();*/

			App->entitycontroller->selected_squads.remove(*it);
			Squad* squad = (*it);
			App->entitycontroller->squads.remove(*it);

			RELEASE(squad);
		}
	}

	//CLEANING ENTITY LISTS---------------------------------------------------
	App->entitycontroller->entities_to_destroy.clear();
	App->entitycontroller->entities.clear();
	App->entitycontroller->selected_entities.clear();
	App->entitycontroller->squads.clear();
	App->entitycontroller->selected_squads.clear();/*
	App->entitycontroller->entity_iterator = App->entitycontroller->entities.begin();
	App->entitycontroller->squad_iterator = App->entitycontroller->squads.begin();*/

	//SATARTING ENTITIES-------------------------------------------------------
	App->entitycontroller->hero = App->entitycontroller->addHero(iPoint(2000, 1950), HERO_1);
	iPoint town_hall_pos = TOWN_HALL_POS;
	Building* town_hall = App->entitycontroller->addBuilding(town_hall_pos, TOWN_HALL);
	App->map->WalkabilityArea(town_hall_pos.x, town_hall_pos.y, town_hall->size.x, town_hall->size.y, true, false);
	App->wavecontroller->updateFlowField();

	//RESTARTING WAVES---------------------------------------------------------
	App->gui->Chronos->counter.Restart();
	App->wavecontroller->Restart_Wave_Sys();
	App->gui->nextWaveWindow->timer->start_value = 0;
	App->gui->nextWaveWindow->timer->setStartValue(App->wavecontroller->initial_wait);

	//RESTARTING RESOURCES-----------------------------------------------------
	wood = INIT_WOOD;
	gold = INIT_GOLD;
	//inactive_workers = workers = INIT_WORKERS;
	InitialWorkers(town_hall);
	town_hall_lvl = INIT_TOWNHALL_LVL;

	//RESTART LOCKED ACTION BUTTONS
	//Hardcoded
	Button* barracks = App->gui->GetActionButton(5);
	barracks->setCondition("Build first a Lumber Mill");
	barracks->Lock();
	Button* farms = App->gui->GetActionButton(7);
	farms->setCondition("Build first a Lumber Mill");
	farms->Lock();

	//CANCEL IF BUILDING
	App->entitycontroller->to_build_type = NONE_ENTITY;
	App->actionscontroller->action_type = NO_ACTION;
	App->actionscontroller->doingAction = false;
	App->actionscontroller->doingAction_lastFrame = false;
	App->gui->warningMessages->hideMessage(NO_WORKERS);
	App->gui->warningMessages->hideMessage(NO_RESOURCES);
	App->gui->warningMessages->hideMessage(NO_TREES);

	//CHANGING MUSIC BACK TO WAVE ONE-----------------------------------------
	App->audio->PlayMusic(MAIN_THEME);

	App->entitycontroller->GetTotalIncome();

	App->gui->newSelectionDone();
	App->uiscene->toggleMenu(false, GAMEOVER_MENU);
	toRestart = false;

}

void j1Scene::InitialWorkers(Building* town_hall)
{
	App->entitycontroller->CreateWorkers(town_hall, 3);
}

void j1Scene::GetTotalInactiveWorkers()
{
	inactive_workers_int = 0;
	for (std::list<worker*>::iterator it = workers.begin(); it != workers.end(); it++)
	{
		if ((*it)->working_at == nullptr)
		{
			inactive_workers_int++;
		}
	}
}
