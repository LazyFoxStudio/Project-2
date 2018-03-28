#ifndef __UI_INGAMEMENU__
#define __UI_INGAMEMENU__

#include "UI_element.h"

class Minimap;
class Window;
class Image;
class HealthBar;
class Button;

class IngameMenu : public UI_element
{
public:

	IngameMenu()
	{}

	IngameMenu(SDL_Texture* texture, int x, int y, SDL_Rect section, int minimap_posX, int minimap_posY, int firstIcon_posX, int firstIcon_posY, int icons_offsetX, int icons_offsetY, int stats_posX, int stats_posY, int firstButton_posX, int firstButton_posY, int buttons_offsetX, int buttons_offsetY, j1Module* callback);

	~IngameMenu()
	{}

	void newSelection();

	void BlitElement(bool use_camera = false);

public:

	Minimap* minimap;
	Window* window;
	iPoint firstIcon_pos;
	iPoint icons_offset;
	iPoint stats_pos;
	iPoint firstButton_pos;
	iPoint buttons_offset;
	std::list<Image*> troopsIcons; //Being created/destroyed
	std::list<HealthBar*> healthBars; //Being created/destroyed
	std::list<Button*> actionButtons; //Constant but unactive

};

#endif // !__UI_INGAMEMENU__