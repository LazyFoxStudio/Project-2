#ifndef __j1SCENE_H__
#define __j1SCENE_H__

#include "j1Module.h"
#include "j1Timer.h"

struct SDL_Texture;
class Window;
class LifeBar;
class Building;


class j1Scene : public j1Module
{
public:

	j1Scene();

	// Destructor
	virtual ~j1Scene();

	// Called before render is available
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

	bool workerAvalible(int num = 1);

	int random_value(int min, int max);
	void Restart_game();

public:
	SDL_Texture* debug_tex = nullptr;

	int wood = 0;
	int gold = 0;
	int oil = 0;
	int workers = 0;
	int inactive_workers = 0;
	int town_hall_lvl = 0;
	
	Building* Town_Hall=nullptr;


};

#endif // __j1SCENE_H__