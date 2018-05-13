
#include <iostream> 
#include <sstream>

#include "p2Defs.h"
#include "p2Log.h"

#include "j1Window.h"
#include "j1Input.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "j1Audio.h"
#include "j1UIScene.h"
#include "j1Scene.h"
#include "j1SceneSwitch.h"
#include "j1Map.h"
#include "j1App.h"
#include "j1EntityController.h"
#include "j1ParticleController.h"
#include "j1ActionsController.h"
#include "j1Pathfinding.h"
#include "j1Fonts.h"
#include "j1Gui.h"
#include "j1Console.h"
#include "j1WaveController.h"
#include "UI_Chrono.h"
#include "UI_NextWaveWindow.h"
#include "j1Tutorial.h"

#include <time.h>

// Constructor
j1App::j1App(int argc, char* args[]) : argc(argc), args(args)
{
	PERF_START(ptimer);

	input				= new j1Input();
	win					= new j1Window();
	render				= new j1Render();
	tex					= new j1Textures();
	audio				= new j1Audio();
	console				= new j1Console();
	uiscene			    = new j1UIScene();
	scene				= new j1Scene();
	sceneswitch			= new j1SceneSwitch();
	map					= new j1Map();
	entitycontroller	= new j1EntityController();
	particle			= new j1ParticleController();
	actionscontroller	= new j1ActionsController();
	pathfinding			= new j1PathFinding();
	font				= new j1Fonts();
	gui					= new j1Gui();
	wavecontroller		= new j1WaveController();
	tutorial			= new j1Tutorial();
	
	// Ordered for awake / Start / Update
	// Reverse order of CleanUp
	AddModule(input);
	AddModule(win);
	AddModule(tex);
	AddModule(audio);
	AddModule(pathfinding);
	AddModule(console);
	AddModule(map);
	AddModule(font);
	AddModule(scene, false);
	AddModule(entitycontroller, false);
	AddModule(tutorial);
	AddModule(gui);
	AddModule(uiscene);
	AddModule(sceneswitch);
	AddModule(particle);
	AddModule(actionscontroller);
	
	AddModule(wavecontroller, false);

	// render last to swap buffer
	AddModule(render);

	PERF_PEEK(ptimer);
}

// Destructor
j1App::~j1App()
{
	for (std::list<j1Module*>::reverse_iterator it = modules.rbegin(); it != modules.rend(); it++) RELEASE(*it);
	modules.clear();
}

void j1App::AddModule(j1Module* module, bool init)
{
	module->active = init;
	modules.push_back(module);
}

// Called before render is available
bool j1App::Awake()
{
	PERF_START(ptimer);
	LOG("Awake:");

	save_game = "save_file.xml";
	load_game = "save_file.xml";
	pugi::xml_document doc;
	pugi::xml_node config = LoadFile(doc, "config.xml");

	if (!config.empty())
	{
		// self-config
		title = config.child("app").child("title").child_value();
		organization = config.child("app").child("organization").child_value();
		int cap = config.child("app").attribute("framerate_cap").as_int(0);

		if (cap) framerate = 1000 / cap;

		for (std::list<j1Module*>::iterator it = modules.begin(); it != modules.end(); it++)
			if(!(*it)->Awake(config.child((*it)->name.c_str()))) return false;
		

	}

	PERF_PEEK(ptimer);
	return true;
}

// Called before the first frame
bool j1App::Start()
{
	PERF_START(ptimer);
	LOG("Start:");

	srand(time(NULL));

	for (std::list<j1Module*>::iterator it = modules.begin(); it != modules.end(); it++)
	{
		if ((*it)->Start())  LOG("name: %s", (*it)->name.c_str());
		else return false;
	}
	PERF_PEEK(ptimer);

	return true;
}

// Called each loop iteration
bool j1App::Update()
{
	BROFILER_CATEGORY("Update", Profiler::Color::Fuchsia);
	bool ret = true;
	PrepareUpdate();
	int test = (int)input->GetWindowEvent(WE_QUIT);
//	LOG("TEST: %d", test);

	if(!input->GetWindowEvent(WE_QUIT))
		if(PreUpdate())
			if(DoUpdate())
				if (PostUpdate())
				{
					FinishUpdate();
					return true;
				}

	FinishUpdate();
	return false;
}

// ---------------------------------------------
pugi::xml_node j1App::LoadFile(pugi::xml_document& doc, char* file) const
{
	pugi::xml_node ret;
	pugi::xml_parse_result result = doc.load_file(file);

	if (result)	ret = doc.first_child();
	else		LOG("Could not load map xml file config.xml. pugi error: %s", result.description());
		
	return ret;
}


// ---------------------------------------------
void j1App::PrepareUpdate()
{
	frame_count++;
	last_sec_frame_count++;

	DeltaTime = frame_time.ReadSec();
//	LOG("DELTA TIME: %f", DeltaTime);
	frame_time.Start();
	ptimer.Start();
}

// ---------------------------------------------
void j1App::FinishUpdate()
{
	if(want_to_save) SavegameNow();
	if(want_to_load) LoadGameNow();

	if (last_sec_frame_time.ReadMs() > 1000)
	{
		last_sec_frame_time.Start();
		prev_last_sec_frame_count = last_sec_frame_count;
		last_sec_frame_count = 0;
	}

	float avg_fps = float(frame_count) / startup_time.ReadSec();
	float seconds_since_startup = startup_time.ReadSec();
	uint32 last_frame_ms = frame_time.ReadMs();
	uint32 frames_on_last_update = prev_last_sec_frame_count;

	static char title[256];
	sprintf_s(title, 256, "Alliance The last Bastion: FPS: %i Avg.FPS: %.2f last frame ms: %02u", frames_on_last_update, avg_fps, last_frame_ms);
	App->win->SetTitle(title);

	if (framerate && last_frame_ms < framerate && fpsCapON) SDL_Delay(framerate - last_frame_ms);

}

// Call modules before each loop iteration
bool j1App::PreUpdate()
{
	BROFILER_CATEGORY("PreUpdate", Profiler::Color::Blue);

	for (std::list<j1Module*>::iterator it = modules.begin(); it != modules.end(); it++)
	{
		if ((*it)->active && (!(*it)->pausable || !paused))
			if (!(*it)->PreUpdate()) return false;
	}

	return true;
}

// Call modules on each loop iteration
bool j1App::DoUpdate()
{
	BROFILER_CATEGORY("DoUpdate", Profiler::Color::LightBlue);

	for (std::list<j1Module*>::iterator it = modules.begin(); it != modules.end(); it++)
	{
		if ((*it)->active && (!(*it)->pausable || !paused))
			if (!(*it)->Update(DeltaTime)) return false;
	}

	return true;
}

// Call modules after each loop iteration
bool j1App::PostUpdate()
{
	BROFILER_CATEGORY("PostUpdate", Profiler::Color::Magenta);

	for (std::list<j1Module*>::iterator it = modules.begin(); it != modules.end(); it++)
	{
		if ((*it)->active && (!(*it)->pausable || !paused))
			if (!(*it)->PostUpdate()) return false;
	}

	return true;
}

// Called before quitting
bool j1App::CleanUp()
{
	PERF_START(ptimer);

	cleaning = true;

	for (std::list<j1Module*>::reverse_iterator it = modules.rbegin(); it != modules.rend(); it++)
		if (!(*it)->CleanUp()) return false;
	

	PERF_PEEK(ptimer);
	return true;
}



// ---------------------------------------
const char* j1App::GetArgv(int index) const
{
	if(index < argc) return args[index];
	else return nullptr;
}

// Load / Save
void j1App::LoadGame()
{
	// we should be checking if that file actually exist
	// from the "GetSaveGames" list
	want_to_load = true;
}

// ---------------------------------------
void j1App::SaveGame() const
{
	// we should be checking if that file actually exist
	// from the "GetSaveGames" list ... should we overwrite ?

	want_to_save = true;
}


bool j1App::LoadGameNow()
{
	bool ret = false;

	pugi::xml_document data;
	pugi::xml_node root;

	pugi::xml_parse_result result = data.load_file(load_game.c_str());

	if(ret = result)
	{
		LOG("Loading new Game State from %s...", load_game.c_str());
		root = data.child("game_state");

		for (std::list<j1Module*>::iterator it = modules.begin(); it != modules.end() && ret; it++)
			ret = (*it)->Load(root.child((*it)->name.c_str()));

		data.reset();
		if(ret) LOG("...finished loading");
		else    LOG("...loading process interrupted with errors");
	}
	else LOG("Could not parse game state xml file %s. pugi error: %s", load_game.c_str(), result.description());

	want_to_load = false;
	return ret;
}

bool j1App::SavegameNow() const
{
	bool ret = true;

	LOG("Saving Game State to %s...", save_game.c_str());

	// xml object were we will store all data
	pugi::xml_document data;
	pugi::xml_node root;
	
	root = data.append_child("game_state");

	for (std::list<j1Module*>::const_iterator it = modules.begin(); it != modules.end() && ret; it++)
		if((*it)->active) ret =  (*it)->Save(root.append_child((*it)->name.c_str()));


	if(ret)
	{
		data.save_file(save_game.c_str());
		LOG("... finished saving", );
	}
	else LOG("Save process halted from an errors");

	data.reset();
	want_to_save = false;
	return ret;
}

void j1App::pauseGame()
{
	paused = true;
	gameTime.PauseTimer();
}

void j1App::resumeGame()
{
	paused = false;
	gameTime.Start();
}

bool j1App::isPaused() const
{
	return paused;
}

bool j1App::isCleaning() const
{
	return cleaning;
}




