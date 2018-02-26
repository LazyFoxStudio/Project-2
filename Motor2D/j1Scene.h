#ifndef __j1SCENE_H__
#define __j1SCENE_H__

#include "j1Module.h"
#include "j1Timer.h"

struct SDL_Texture;
class Window;
class LifeBar;

enum ListOfMapNames
{
	OuterWorld = 0,
	DepthsOfTheAbyss
}; 

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

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	bool OnEvent(UIElement* element, int eventType);

	bool Load(pugi::xml_node& data);
	bool Save(pugi::xml_node& data) const;

	bool Load_lvl(int time);
	void OpenPauseMenu();
	void ClosePauseMenu();
	void SpawnEnemies();


public:
	LifeBar* lifebar = nullptr;
	Window* sceneMenu = nullptr;
	SDL_Texture* debug_tex;
	iPoint test;
	bool slowing = false;
	int backToNormal = -1;
	int currentMap;
	int currentTime = 0;
	int pastFrameTime = 0;
	int playerLives = 4;
	p2List<p2SString*> map_names;
	float fade_time;
	bool to_end;
	j1Timer transcurredTime;
	bool pause = false;
};

#endif // __j1SCENE_H__