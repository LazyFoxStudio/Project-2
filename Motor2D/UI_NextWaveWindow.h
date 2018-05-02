#ifndef __UI_NEXTWAVEWINDOW_H__
#define __UI_NEXTWAVEWINDOW_H__

#include "UI_element.h"
#include "UI_SelectionDisplay.h"

class Chrono;
class Text;
class Window;
class SDL_texture;
class Button;

class NextWaveWindow : public UI_element
{
public:
	NextWaveWindow(SDL_Texture* atlas, SDL_Texture* icon_atlas, Button* button, int x, int y, int min_x, int min_y, SDL_Rect section, int firstIcon_posX, int firstIcon_posY, int icons_offsetX, int icons_offsetY, j1Module* callback);
	~NextWaveWindow();

	void BlitElement();

	void updateWave();
	bool createIncomingEnemy(Type type, int amount);
	void cleanIcons();
	void toggle();

	UI_element* getMouseHoveringElement();

public:

	SDL_Texture* icon_atlas = nullptr;
	Window* window = nullptr;
	std::list<TroopIcon*> enemiesIcons;
	std::list<Text*> squads;
	Text* text = nullptr;
	Chrono* timer = nullptr;
	Button* button = nullptr;
	bool minimized = false;

	int counterX = 0;
	int counterY = 0;

	iPoint firstIcon_pos = { 0,0 };
	iPoint icons_offset = { 0,0 };
	iPoint default_pos = { 0,0 };
	iPoint minimized_pos = { 0,0 };

};

#endif // _UI_NEXTWAVEWINDOW_H__