#ifndef __UI_INGAMEMENU__
#define __UI_INGAMEMENU__

#include "UI_element.h"

class Minimap;
class Image;
class HealthBar;
class Button;

class IngameMenu : public UI_element
{
public:

	IngameMenu()
	{}

	IngameMenu(SDL_Texture* texture, int x, int y, SDL_Rect section, j1Module* callback): UI_element(x, y, element_type::MENU, section, callback, texture)
	{}

	~IngameMenu()
	{}

	void newSelection();

	void BlitElement(bool use_camera = false);

public:

	Minimap* minimap;
	Image* background;
	std::list<Image*> troopsIcons; //Being created/destroyed
	std::list<HealthBar*> healthBars; //Being created/destroyed
	std::list<Button*> actionButtons; //Constant but unactive

};

#endif // !__UI_INGAMEMENU__