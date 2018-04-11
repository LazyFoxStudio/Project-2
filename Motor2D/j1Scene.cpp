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
#include "UI_CostDisplay.h"

j1Scene::j1Scene() : j1Module() { name = "scene"; }

// Destructor
j1Scene::~j1Scene() {}

// Called before render is available
bool j1Scene::Awake(pugi::xml_node& config)
{
	LOG("Loading Scene");
	wood = config.child("starting_resources").child("wood").attribute("value").as_int(0);
	gold = config.child("starting_resources").child("gold").attribute("value").as_int(0);
	workers = config.child("starting_resources").child("workers").attribute("value").as_int(0);
	inactive_workers = workers;
	town_hall_lvl = config.child("starting_resources").child("townHallLvl").attribute("value").as_int(0);
	return true;
}

// Called before the first frame
bool j1Scene::Start()
{
	int w = -1, h = -1;
	uchar* data = nullptr;

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

	return true;
}

// Called each loop iteration
bool j1Scene::Update(float dt)
{
	BROFILER_CATEGORY("Scene update", Profiler::Color::Black);

	if (App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN && !App->entitycontroller->building)
	{
		return false;
	}
	App->render->MouseCameraMovement(dt);
	App->map->Draw();


	if (App->input->GetKey(SDL_SCANCODE_P) == KEY_REPEAT)
	{
		App->audio->PlayFx(listOfSFX::PEASANT_READY);
	}
	if (App->input->GetKey(SDL_SCANCODE_O) == KEY_REPEAT)
	{
		App->audio->PlayFx(listOfSFX::PEASANT_FOLLOW_ORDERS);
	}

	return true;
}

// Called each loop iteration
bool j1Scene::PostUpdate()
{
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
