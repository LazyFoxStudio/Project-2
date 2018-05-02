#ifndef __j1SCENE_H__
#define __j1SCENE_H__

#include "j1Module.h"
#include "j1Timer.h"

struct SDL_Texture;
class Window;
class LifeBar;
class Building;
struct worker;

#define INIT_WOOD 50000
#define INIT_GOLD 500
#define INIT_OIL 100
#define INIT_WORKERS 3

#define INIT_TOWNHALL_LVL 0

class j1Scene : public j1Module
{
public:

	j1Scene();

	// Destructor
	virtual ~j1Scene();

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	bool Load(pugi::xml_node& data);
	bool Save(pugi::xml_node& data) const;

	//bool workerAvalible(int num = 1);

	void Restart_game();
	void InitialWorkers(Building* town_hall);
	void GetTotalInactiveWorkers();

	void loadGameDB(pugi::xml_node& data);

public:

	j1Timer Restart_timer;
	bool toRestart=false;
	int restart_time = 0;

	int wood = 0;
	int gold = 0;
	int oil = 0;

	std::list<worker*> workers;
	int workers_int = 0;
	int inactive_workers_int = 0;
	//int workers = 0;


	int town_hall_lvl = 0;
	
	int wood_production_per_second = 0;
};

#endif // __j1SCENE_H__