#ifndef __j1SCENE_H__
#define __j1SCENE_H__

#include "j1Module.h"
#include "j1Timer.h"
#include "j1Console.h"

struct SDL_Texture;
class Window;
class LifeBar;
class Building;
struct worker;


#define INIT_WOOD 50000
#define INIT_GOLD 50000
#define INIT_OIL 100
#define INIT_WORKERS 3

#define INIT_TOWNHALL_LVL 0

class j1Scene : public j1Module
{
public:

	j1Scene();

	// Destructor
	virtual ~j1Scene();

	bool Awake(pugi::xml_node&);

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

	void Close_game();
	void Start_game();
	void InitialWorkers(Building* town_hall);
	void GetTotalInactiveWorkers();
	void DeleteWorkers();

	void loadGameDB(pugi::xml_node& data);

	bool Console_Interaction(std::string& function, std::vector<int>& arguments);

public:

	j1Timer Restart_timer;
	bool toRestart=false;
	int restart_time = 0;

	int wood = 0;
	int gold = 0;
	int oil = 0;
	
	int i_wood = 0;
	int i_gold = 0;
	int i_oil = 0;
	int i_workers = 0;

	std::list<worker*> workers;
	int workers_int = 0;
	int inactive_workers_int = 0;
	//int workers = 0;


	int town_hall_lvl = 0;
	
	int wood_production_per_second = 0;
	int gold_production_per_second = 0;

	function* set_wood_second;
	function* set_gold_second;
	function* set_wood;
	function* set_gold;

};

#endif // __j1SCENE_H__