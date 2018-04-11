#ifndef __UI_NEXTWAVEWINDOW_H__
#define __UI_NEXTWAVEWINDOW_H__

#include "UI_element.h"
#include "UI_IngameMenu.h"

class Chrono;
class Text;
class Window;

class NextWaveWindow : public UI_element
{
public:
	NextWaveWindow(SDL_Texture* atlas, int x, int y, SDL_Rect section, int firstIcon_posX, int firstIcon_posY, int icons_offsetX, int icons_offsetY, j1Module* callback);
	~NextWaveWindow();

	void BlitElement(bool use_camera = false);

	void updateWave();

public:

	Window* window = nullptr;
	std::list<TroopIcon*> enemiesIcons;
	Text* text = nullptr;
	Chrono* timer = nullptr;

	iPoint firstIcon_pos;
	iPoint icons_offset;

};

#endif // _UI_NEXTWAVEWINDOW_H__