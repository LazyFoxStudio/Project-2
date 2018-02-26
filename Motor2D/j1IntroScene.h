#ifndef __j1INTROSCENE_H__
#define __j1INTROSCENE_H__

#include "j1Module.h"

class UIElement;
class Window;
struct SDL_Texture;

class j1IntroScene : public j1Module
{
public:
	j1IntroScene();
	virtual ~j1IntroScene();

	bool Awake();

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

};

#endif
