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

	~LifeBar()
	{	}

	void BlitElement(bool use_camera = false);

public:

	ProgressBar* bar;
	Entity* entity;

};

#endif // !__UI_LIFEBAR__
