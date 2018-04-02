#ifndef __UI_LIFEBAR__
#define __UI_LIFEBAR__

#include "UI_element.h"

class progressBar;
class Entity;
class Text;

class LifeBar : public UI_element
{
public:

	LifeBar()
	{}

	LifeBar(Entity* entity, SDL_Texture* texture);

	LifeBar(Entity* entity, SDL_Texture* texture, int x, int y);

	~LifeBar();

	void BlitElement(bool use_camera = false);

public:

	ProgressBar* bar = nullptr;
	Entity* entity = nullptr;
	Text* display = nullptr;
	iPoint offset = { 0,0 };
	bool inMenu=false;

};

#endif // !__UI_LIFEBAR__
