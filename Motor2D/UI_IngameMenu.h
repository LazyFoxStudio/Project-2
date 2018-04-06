#ifndef __UI_INGAMEMENU__
#define __UI_INGAMEMENU__

#include "UI_element.h"

class Minimap;
class Window;
class Image;
class LifeBar;
class Button;
class Text;

class IngameMenu : public UI_element
{
public:

	IngameMenu()
	{}

	IngameMenu(SDL_Texture* atlas, SDL_Texture* icon_atlas, int x, int y, SDL_Rect section, int minimap_posX, int minimap_posY, int firstIcon_posX, int firstIcon_posY, int icons_offsetX, int icons_offsetY, int lifeBars_offsetX, int lifeBars_offsetY, int stats_posX, int stats_posY, int firstButton_posX, int firstButton_posY, int buttons_offsetX, int buttons_offsetY, j1Module* callback);

	~IngameMenu();

	void updateInfo();
	//Troops icons
	//Troops life bars
	void createSelectionBasicInfo();
	void createStatsDisplay();
	void updateStatsDisplay();
	//void createActionButtons(pugi::xml_node node);
	void updateActionButtons();
	void cleanLists(bool icons = true, bool lifeBars = true, bool statsTitles = true, bool statsNumbers = true, bool buttons = true);

	void BlitElement(bool use_camera = false);

	UI_element* getMouseHoveringElement();

public:

	SDL_Texture* icon_atlas = nullptr;
	Minimap* minimap = nullptr;
	Window* window = nullptr;
	iPoint firstIcon_pos = { 0,0};
	iPoint icons_offset = { 0,0};
	iPoint lifeBars_offset = { 0,0};
	iPoint stats_pos = { 0,0};
	iPoint firstButton_pos = { 0,0 };
	iPoint buttons_offset = { 0,0 };
	std::list<Image*> troopsIcons; //Being created/destroyed
	std::list<LifeBar*> lifeBars; //Being created/destroyed
	std::list<Text*> statsTitles; //Constant but inactive
	Text* title=nullptr; //Constant but inactive
	std::list<Text*> statsNumbers; //Constant but inactive
	std::list<Button*> actionButtons; //Loading the activated buttons

};

#endif // !__UI_INGAMEMENU__