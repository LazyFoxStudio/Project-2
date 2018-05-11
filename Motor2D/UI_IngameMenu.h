#ifndef __UI_INGAMEMENU__
#define __UI_INGAMEMENU__

#include "UI_element.h"

#define MAX_ACTION_BUTONS 9
#define DEFAULT_HOTKEY_1 SDL_SCANCODE_Q
#define DEFAULT_HOTKEY_2 SDL_SCANCODE_W
#define DEFAULT_HOTKEY_3 SDL_SCANCODE_E
#define DEFAULT_HOTKEY_4 SDL_SCANCODE_A
#define DEFAULT_HOTKEY_5 SDL_SCANCODE_S
#define DEFAULT_HOTKEY_6 SDL_SCANCODE_D
#define DEFAULT_HOTKEY_7 SDL_SCANCODE_Z
#define DEFAULT_HOTKEY_8 SDL_SCANCODE_X
#define DEFAULT_HOTKEY_9 SDL_SCANCODE_C

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

	void BlitElement();

	UI_element* getMouseHoveringElement();

public:

	SelectionDisplay* selectionDisplay = nullptr;
	InfoTable* infoTable = nullptr;
	Window* window = nullptr;
	iPoint firstButton_pos = { 0,0 };
	iPoint buttons_offset = { 0,0 };
	Text* workers_title = nullptr;
	Text* workers = nullptr;
	std::list<Button*> actionButtons; //Loading the activated buttons

	std::vector<SDL_Scancode> actionButtonsHotkeys;

};

#endif // !__UI_INGAMEMENU__