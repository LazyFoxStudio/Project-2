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
#include "j1ParticleController.h"
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
#include "j1Tutorial.h"
#include "UI_UnlockDisplay.h"

j1Scene::j1Scene() : j1Module() { name = "scene"; pausable = false; }

// Destructor
j1Scene::~j1Scene() {}

bool j1Scene::Awake(pugi::xml_node& conf)
{
	LOG("Loading GUI atlas");
	bool ret = true;

	i_wood = conf.child("Wood").attribute("value").as_int(0);
	i_gold = conf.child("Gold").attribute("value").as_int(0);
	i_workers = conf.child("Workers").attribute("value").as_int(0);
	i_oil = conf.child("Oil").attribute("value").as_int(0);

	return ret;
}

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

	App->render->culling_camera.x = -App->render->camera.x - 50;
	App->render->culling_camera.y = -App->render->camera.y - 50;

	restart_time = 10;
	//wood = INIT_WOOD;
	//gold = INIT_GOLD;
	//inactive_workers = workers = INIT_WORKERS;
	//town_hall_lvl = INIT_TOWNHALL_LVL;

	pugi::xml_document doc;
	pugi::xml_node gameData;

	gameData = App->LoadFile(doc, "GameData.xml");
	loadGameDB(gameData);
	//App->entitycontroller->addUnit({ 2000, 2200 }, KNIGHT);
	//App->entitycontroller->AddSquad(FOOTMAN, { 2000,2200 });

	set_wood = App->console->AddFunction("set_wood", this, 1, 1, "amount");
	set_gold = App->console->AddFunction("set_gold", this, 1, 1, "amount");
	set_wood_second = App->console->AddFunction("set_wood_second", this, 1, 1, "amount");
	set_gold_second = App->console->AddFunction("set_gold_second", this, 1, 1, "amount");

	return true;
}

// Called each loop iteration
bool j1Scene::Update(float dt)
{
	BROFILER_CATEGORY("Scene update", Profiler::Color::Chocolate);
	if (!App->isPaused())
		App->render->MouseCameraMovement(dt);
	
	App->map->Draw();

	//Music and SFX modifiers (temporal for Vertical Slice)
	/*if (App->input->GetKey(SDL_SCANCODE_P) == KEY_DOWN)
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
	}*/
	/*if (App->input->GetKey(SDL_SCANCODE_2) == KEY_DOWN)
	{
		App->entitycontroller->UnassignRandomWorker();
	}*/

	workers_int = workers.size();
	GetTotalInactiveWorkers();

	//TEST SPAWNER--------------------------------------------------
	if (App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
	{
		int x, y;
		iPoint mouse_pos;
		App->input->GetMousePosition(x,y);
		mouse_pos=App->render->ScreenToWorld(x, y);
		App->entitycontroller->AddSquad(CATAPULT, {(float)mouse_pos.x,(float)mouse_pos.y});
	}

	if (App->input->GetKey(SDL_SCANCODE_F7) == KEY_DOWN)
	{
		App->SaveGame();
	}
	if (App->input->GetKey(SDL_SCANCODE_F9) == KEY_DOWN)
	{
		App->LoadGame();
	}


	return true;
}

// Called each loop iteration
bool j1Scene::PostUpdate()
{
	if (App->input->GetKey(SDL_SCANCODE_F4) == KEY_DOWN )
		return false;

	if (toRestart && Restart_timer.ReadSec() >= restart_time) Start_game();
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
	gold = data.child("gold").attribute("value").as_int();
	wood = data.child("wood").attribute("value").as_int();
	oil = data.child("oil").attribute("value").as_int();
	return true;
}

bool j1Scene::Save(pugi::xml_node& data) const
{
	data.append_child("gold").append_attribute("value") = gold;
	data.append_child("wood").append_attribute("value") = wood;
	data.append_child("oil").append_attribute("value") = oil;

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

void j1Scene::Close_game()
{
	//DELETING ENTITIES-------------------------------------------------------
	for (std::list<Entity*>::iterator it = App->entitycontroller->entities.begin(); it != App->entitycontroller->entities.end(); it++)
	{
		App->gui->entityDeleted(*it);
		App->entitycontroller->DeleteEntity((*it)->UID);
	}

	for (std::list<Squad*>::iterator it = App->entitycontroller->squads.begin(); it != App->entitycontroller->squads.end(); it++)
		App->entitycontroller->DeleteSquad((*it)->UID);

	App->entitycontroller->last_UID = 0;

	//CLEANING ENTITY LISTS---------------------------------------------------
	App->entitycontroller->selected_entities.clear();
	App->entitycontroller->selected_squads.clear();
	App->entitycontroller->entities.clear();
	App->entitycontroller->squads.clear();

	if (App->tutorial->active)
		App->tutorial->stopTutorial();
}

void j1Scene::Start_game(bool continuing)
{
	App->pauseGame();

	if (!App->tutorial->active)
		Close_game();

	if (App->tutorial->active && !continuing)
	{
		App->render->camera.x = 0;
		App->render->camera.y = -3200;
		App->render->culling_camera.x = -App->render->camera.x - 50;
		App->render->culling_camera.y = -App->render->camera.y - 50;
	}
	else
	{
		App->render->camera.x = -1200;
		App->render->camera.y = -1600;
		App->render->culling_camera.x = -App->render->camera.x - 50;
		App->render->culling_camera.y = -App->render->camera.y - 50;
	}

	//SATARTING ENTITIES-------------------------------------------------------
	
	pugi::xml_document doc;
	pugi::xml_node gameData;

	App->entitycontroller->DeleteDB();
		gameData = App->LoadFile(doc, "GameData.xml");
		App->entitycontroller->loadEntitiesDB(gameData);

	iPoint town_hall_pos = TOWN_HALL_POS;
	App->entitycontroller->town_hall = App->entitycontroller->addBuilding(town_hall_pos, TOWN_HALL);
	App->map->WalkabilityArea(town_hall_pos.x, town_hall_pos.y, App->entitycontroller->town_hall->size.x, App->entitycontroller->town_hall->size.y, true, false);
	App->wavecontroller->updateFlowField();
	App->entitycontroller->buildingArea.w = BUILDINGAREA;
	App->entitycontroller->buildingArea.h = BUILDINGAREA;
	//buildingArea.x = -BUILDINGAREA / 2 + town_hall_pos.x / 2;
	//buildingArea.y = -BUILDINGAREA / 2 + town_hall_pos.y / 2;
	App->entitycontroller->buildingArea.x = town_hall_pos.x - (BUILDINGAREA / 2) + (App->entitycontroller->town_hall->size.x*App->map->data.tile_width / 2);
	App->entitycontroller->buildingArea.y = town_hall_pos.y - (BUILDINGAREA / 2) + (App->entitycontroller->town_hall->size.x*App->map->data.tile_height / 2);
/*
	App->entitycontroller->AddSquad(FOOTMAN, fPoint(2000, 2000));
	App->entitycontroller->AddSquad(FOOTMAN, fPoint(2000, 2000));*/
	//RESTARTING WAVES---------------------------------------------------------
	App->gui->Chronos->counter.Restart();
	App->wavecontroller->Restart_Wave_Sys();
	App->gui->nextWaveWindow->timer->start_value = 0;
	App->gui->nextWaveWindow->timer->setStartValue(App->wavecontroller->initial_wait);
	if (App->tutorial->doingTutorial)
	{
		App->wavecontroller->wave_timer.PauseTimer();
		App->gui->nextWaveWindow->active = false;
		App->gui->nextWaveWindow->timer->counter.PauseTimer();
	}

	//RESTARTING RESOURCES-----------------------------------------------------
	wood = i_wood;
	gold = i_gold;
	//inactive_workers = workers = INIT_WORKERS;
	DeleteWorkers();
	InitialWorkers(App->entitycontroller->town_hall);
	town_hall_lvl = INIT_TOWNHALL_LVL;

	//RESTART LOCKED ACTION BUTTONS
	//Hardcoded
	Button* barracks = App->gui->GetActionButton(5);
	barracks->setCondition("Build first a Lumber Mill");
	barracks->Lock();
	Button* farms = App->gui->GetActionButton(7);
	farms->setCondition("Build first a Lumber Mill");
	farms->Lock();
	Button* mine = App->gui->GetActionButton(22);
	mine->setCondition("Build first a Lumber Mill");
	mine->Lock();
	Button* turret = App->gui->GetActionButton(23);
	turret->setCondition("Build first a Farm");
	turret->Lock();
	Button* hut = App->gui->GetActionButton(24);
	hut->setCondition("Build first a Barracks");
	hut->Lock();
	Button* church = App->gui->GetActionButton(25);
	church->setCondition("Build first a Mine");
	church->Lock();
	Button* blacksmith = App->gui->GetActionButton(26);
	blacksmith->setCondition("Build first a Gnome Hut");
	blacksmith->Lock();

	//CANCEL IF BUILDING
	App->entitycontroller->to_build_type = NONE_ENTITY;
	App->actionscontroller->action_type = NO_ACTION;
	App->actionscontroller->doingAction = false;
	App->actionscontroller->doingAction_lastFrame = false;
	App->gui->warningMessages->hideMessage(NO_WORKERS);
	App->gui->warningMessages->hideMessage(NO_RESOURCES);
	App->gui->warningMessages->hideMessage(NO_TREES);

	//CHANGING MUSIC BACK TO WAVE ONE-----------------------------------------
	App->audio->PlayMusic(INGAME_THEME, 0);

	App->entitycontroller->GetTotalIncome();

	App->gui->newSelectionDone();
	App->uiscene->toggleMenu(false, GAMEOVER_MENU);
	if (!continuing)
	{
		if (App->tutorial->active)
			App->uiscene->toggleMenu(true, SKIP_TUTORIAL_MENU);
		else
			App->uiscene->toggleMenu(true, HERO_SELECTION_MENU);
	}

	App->gui->unlockDisplay->cleanUnlockments();

	toRestart = false;
}

void j1Scene::InitialWorkers(Building* town_hall)
{
	App->entitycontroller->CreateWorkers(town_hall, i_workers);
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

void j1Scene::loadGameDB(pugi::xml_node& data)
{
	//Load Life Bars
	App->gui->LoadLifeBarsDB(data);
	//Load fonts
	App->gui->LoadFonts(data.child("fonts"));

	//Load entities DB
	App->entitycontroller->loadEntitiesDB(data);

	//Load action buttons
	App->gui->LoadActionButtonsDB(data);

	//Load workers display
	App->gui->LoadWorkersDisplayDB(data);
}

bool j1Scene::Console_Interaction(std::string& function, std::vector<int>& arguments)
{
	if (function == set_gold->name)
	{
		gold = arguments.data()[0];
	}

	if (function == set_wood->name)
	{
		wood = arguments.data()[0];

	}
	if (function == set_gold_second->name)
	{
		gold_production_per_second = arguments.data()[0];

	}
	if (function == set_wood_second->name)
	{
		wood_production_per_second = arguments.data()[0];
	}

	return true;
}

void j1Scene::DeleteWorkers()
{
	workers.clear();
}