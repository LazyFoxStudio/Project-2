#ifndef __UI_INGAMEMENU__
#define __UI_INGAMEMENU__

#include "UI_element.h"

class Window;
class Button;
class Text;
class SelectionDisplay;
class InfoTable;

class IngameMenu : public UI_element
{
public:

	IngameMenu()
	{}

	IngameMenu(SDL_Texture* atlas, int x, int y, SDL_Rect section, int firstButton_posX, int firstButton_posY, int buttons_offsetX, int buttons_offsetY, j1Module* callback);

	~IngameMenu();

	void updateInfo();

	void updateActionButtons();

	void BlitElement(bool use_camera = false);

	UI_element* getMouseHoveringElement();

public:

	SelectionDisplay* selectionDisplay = nullptr;
	InfoTable* infoTable = nullptr;
	Window* window = nullptr;
	iPoint firstButton_pos = { 0,0 };
	iPoint buttons_offset = { 0,0 };
	Text* workers = nullptr;
	std::list<Button*> actionButtons; //Loading the activated buttons

};

#endif // !__UI_INGAMEMENU__