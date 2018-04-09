#ifndef __j1APP_H__
#define __j1APP_H__

#include "Brofiler/Brofiler.h"
#include "j1Module.h"
#include "j1PerfTimer.h"
#include "j1Timer.h"
#include "PugiXml\src\pugixml.hpp"

#include <list>
#include <vector>
#include <string>

// Modules
class j1Window;
class j1Input;
class j1Render;
class j1Textures;
class j1Audio;
class j1UIScene;
class j1Scene;
class j1SceneSwitch;
class j1Map;
class j1EntityController;
class j1ActionsController;
class j1PathFinding;
class j1Fonts;
class j1Gui;
class j1Console;
class j1WaveController;

class j1App
{
public:

	// Constructor
	j1App(int argc, char* args[]);

	// Destructor
	virtual ~j1App();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool Update();

	// Called before quitting
	bool CleanUp();

	// Add a new module to handle
	void AddModule(j1Module* module, bool init = true);

	// Exposing some properties for reading
	int GetArgc() const					{ return argc; };
	const char* GetTitle() const		{ return title.c_str(); };
	const char* GetOrganization() const { return organization.c_str(); };
	const char* GetArgv(int index) const;

	void LoadGame();
	void SaveGame() const;

	// Load config file
	pugi::xml_node LoadFile(pugi::xml_document&, char* file) const;

private:

	

	// Call modules before each loop iteration
	void PrepareUpdate();

	// Call modules before each loop iteration
	void FinishUpdate();

	// Call modules before each loop iteration
	bool PreUpdate();

	// Call modules on each loop iteration
	bool DoUpdate();

	// Call modules after each loop iteration
	bool PostUpdate();

	// Load / Save
	bool LoadGameNow();
	bool SavegameNow() const;


public:

	// Modules
	j1Window*				win = nullptr;
	j1Input*				input = nullptr;
	j1Render*				render = nullptr;
	j1Textures*				tex = nullptr;
	j1Audio*				audio = nullptr;
	j1UIScene*				uiscene = nullptr;
	j1Console*				console = nullptr;
	j1Scene*				scene = nullptr;
	j1SceneSwitch*			sceneswitch = nullptr;
	j1Map*					map = nullptr;
	j1EntityController*		entitycontroller = nullptr;
	j1ActionsController*	actionscontroller = nullptr;
	j1PathFinding*			pathfinding = nullptr;
	j1Fonts*				font = nullptr;
	j1Gui*					gui = nullptr;
	j1WaveController* wavecontroller = nullptr;

	bool fpsCapON = true;
	std::string			load_game;

private:

	std::list<j1Module*>	modules;
	int						argc=0;
	char**					args = nullptr;

	std::string			title="";
	std::string			organization="";

	mutable bool		want_to_save = false;
	bool				want_to_load = false;

	mutable std::string	save_game;

	j1PerfTimer			ptimer;
	uint64				frame_count = 0;
	j1Timer				startup_time;
	j1Timer				frame_time;
	j1Timer				last_sec_frame_time;
	uint32				last_sec_frame_count = 0;
	uint32				prev_last_sec_frame_count = 0;
	float				DeltaTime = 0.0f;

public:
	uint16_t			framerate = 0;

};

extern j1App* App; // No student is asking me about that ... odd :-S

#endif