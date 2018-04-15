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
#include "j1EntityController.h"
#include "j1WaveController.h"
#include "UI_CostDisplay.h"
#include "UI_Chrono.h"
#include "UI_NextWaveWindow.h"
#include "Building.h"


j1Scene::j1Scene() : j1Module() { name = "scene"; }

// Destructor
j1Scene::~j1Scene() {}

// Called before render is available
bool j1Scene::Awake(pugi::xml_node& config)
{
	LOG("Loading Scene");
	init_wood = wood = config.child("starting_resources").child("wood").attribute("value").as_int(0);
	init_gold = gold = config.child("starting_resources").child("gold").attribute("value").as_int(0);
	init_workers = workers = config.child("starting_resources").child("workers").attribute("value").as_int(0);
	inactive_workers = workers;
	town_hall_lvl = config.child("starting_resources").child("townHallLvl").attribute("value").as_int(0);
	return true;
}

// Called before the first frame
bool j1Scene::Start()
{
	int w = -1, h = -1;
	uchar* data = nullptr;

	App->audio->PlayMusic("Normal_Round_Theme.ogg");
	App->map->Load_map("map1.tmx");

	if (App->map->CreateWalkabilityMap(w, h, &data))	
		App->pathfinding->SetMap(w, h, data);

	debug_tex = App->tex->Load("maps/Navigable.png");

	pugi::xml_document	Gui_config_file;
	pugi::xml_node		guiconfig;

	guiconfig = App->LoadFile(Gui_config_file, "Gui_config.xml");
	guiconfig = guiconfig.child("scene");

	App->render->camera.x = -1200;
	App->render->camera.y = -1600;

	restart_time = 10;

	return true;
}

// Called each loop iteration
bool j1Scene::Update(float dt)
{
	BROFILER_CATEGORY("Scene update", Profiler::Color::Chocolate);

	if (App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN && !App->entitycontroller->building)
	{
		return false;
	}
	
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


	return true;
}

// Called each loop iteration
bool j1Scene::PostUpdate()
{
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

bool j1Scene::workerAvalible(int num)
{
	bool ret = false;

	if (inactive_workers == 0)
	{
		ret = false;
	}
	else if (inactive_workers >= num)
	{
		ret = true;
	}

	return ret;
}

void j1Scene::Restart_game()
{
	if (Restart_timer.ReadSec() >= restart_time)
	{
		//DELETING ENTITIES-------------------------------------------------------
		std::list<Entity*>::iterator it = App->entitycontroller->entities.begin();
		while (it != App->entitycontroller->entities.end())
		{
			App->entitycontroller->DeleteEntity(*it);
			it++;
		}

		//CLEANING ENTITY LISTS---------------------------------------------------
		App->entitycontroller->entities_to_destroy.clear();
		App->entitycontroller->entities.clear();
		App->entitycontroller->selected_entities.clear();
		App->entitycontroller->all_squads.clear();
		App->entitycontroller->selected_squads.clear();
		App->entitycontroller->entity_iterator = App->entitycontroller->entities.begin();
		App->entitycontroller->squad_iterator = App->entitycontroller->all_squads.begin();

		//SATARTING ENTITIES-------------------------------------------------------
		App->entitycontroller->building = false;
		App->entitycontroller->placingBuilding(TOWN_HALL, { 2000, 2000 });
		App->entitycontroller->StartHero(iPoint(2000, 1950));

		//RESTARTING WAVES---------------------------------------------------------
		App->gui->Chronos->counter.Restart();
		App->wavecontroller->Restart_Wave_Sys();
		App->gui->nextWaveWindow->timer->start_value = 0;
		App->gui->nextWaveWindow->timer->setStartValue(App->wavecontroller->initial_wait);

		//RESTARTING RESOURCES-----------------------------------------------------
		wood = init_wood;
		gold = init_gold;
		workers = init_workers;
		inactive_workers = workers;

		//CHANGING MUSIC BACK TO WAVE ONE-----------------------------------------
		App->audio->PlayMusic("Normal_Round_Theme.ogg");


		App->uiscene->toggleMenu(false, GAMEOVER_MENU);
		toRestart = false;
	}
}
