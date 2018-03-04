#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Input.h"
#include "j1Textures.h"
#include "j1Audio.h"
#include "j1Render.h"
#include "j1Window.h"
#include "Window.h"
#include "j1Map.h"
#include "LabelledImage.h"
#include "j1Scene.h"
#include "j1IntroScene.h"
#include "j1Sceneswitch.h"
#include "j1Pathfinding.h"
#include "j1EntityController.h"
#include "j1Gui.h"
#include "LifeBar.h"

j1Scene::j1Scene() : j1Module()
{
	name.create("scene");
}

// Destructor
j1Scene::~j1Scene()
{}

// Called before render is available
bool j1Scene::Awake(pugi::xml_node& config)
{
	LOG("Loading Scene");

	fade_time = config.child("fade_time").attribute("value").as_float();

	for (pugi::xml_node map = config.child("map_name"); map; map = map.next_sibling("map_name"))
	{
		p2SString* data = new p2SString;

		data->create(map.attribute("name").as_string());
		map_names.add(data);
	}
	
	bool ret = true;

	return ret;
}

// Called before the first frame
bool j1Scene::Start()
{

	pugi::xml_document	config_file;
	pugi::xml_node		config;

	config = App->LoadConfig(config_file);

	to_end = false;
	if (!App->map->Load_map(map_names.start->data->GetString()))
	{
		LOG("Error loading map %s", map_names.start->data->GetString());
		return false;
	}

	int w, h;
	uchar* data = NULL;
	if (App->map->CreateWalkabilityMap(w, h, &data))
		App->pathfinding->SetMap(w, h, data);

	debug_tex = App->tex->Load("maps/Navigable.png");


	pugi::xml_document	Gui_config_file;
	pugi::xml_node		guiconfig;

	guiconfig = App->LoadConfig(Gui_config_file, "Gui_config.xml");

	guiconfig = guiconfig.child("scene");

	transcurredTime.Start();
	
	return true;

}

// Called each loop iteration
bool j1Scene::PreUpdate()
{
	return true;
}

// Called each loop iteration
bool j1Scene::Update(float dt)
{
	BROFILER_CATEGORY("Scene update", Profiler::Color::Black);


	App->map->Draw();
	App->gui->Draw(dt);



	return true;
}

// Called each loop iteration
bool j1Scene::PostUpdate()
{
	bool ret = true;


	if (App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
	{
		pause = !pause;
		if (pause)
			OpenPauseMenu();
		else
			ClosePauseMenu();
	}
	return ret;
}

// Called before quitting
bool j1Scene::CleanUp()
{
	currentTime = 0;
	LOG("Freeing scene");

	return true;
}

bool j1Scene::OnEvent(UIElement* element, int eventType)
{
	bool ret = true;

	return ret;
}

bool j1Scene::Load(pugi::xml_node& data)
{

	return true;
}

bool j1Scene::Save(pugi::xml_node& data) const
{
	data.append_child("currentMap").append_attribute("num") = currentMap;
	data.append_child("time").append_attribute("value") = currentTime;


	return true;
}


void j1Scene::OpenPauseMenu()
{
	if(sceneMenu)
	sceneMenu->active = true;
}

void j1Scene::ClosePauseMenu()
{
	if (sceneMenu)
	sceneMenu->active = false;
}
