#ifndef __UI_LIFEBAR__
#define __UI_LIFEBAR__

#include "UI_element.h"

class progressBar;
class Entity;

class LifeBar : public UI_element
{
public:

	LifeBar()
	{}

	LifeBar(Entity* entity, SDL_Texture* texture);

	LifeBar(Entity* entity, SDL_Texture* texture, int x, int y);

	~LifeBar()
	{	}

	void BlitElement(bool use_camera = false);

public:

	ProgressBar* bar;
	Entity* entity;
	iPoint offset;
	bool inMenu;

};

#endif // !__UI_LIFEBAR__
