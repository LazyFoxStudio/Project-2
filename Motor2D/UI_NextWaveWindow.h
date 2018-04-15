#ifndef __UI_NEXTWAVEWINDOW_H__
#define __UI_NEXTWAVEWINDOW_H__

#include "UI_element.h"
#include "UI_IngameMenu.h"

class Chrono;
class Text;
class Window;
class SDL_texture;

class NextWaveWindow : public UI_element
{
public:
	NextWaveWindow(SDL_Texture* atlas, SDL_Texture* icon_atlas, int x, int y, SDL_Rect section, int firstIcon_posX, int firstIcon_posY, int icons_offsetX, int icons_offsetY, j1Module* callback);
	~NextWaveWindow();

	void BlitElement(bool use_camera = false);

	void updateWave();
	void cleanIcons();

public:

	SDL_Texture* icon_atlas = nullptr;
	Window* window = nullptr;
	std::list<TroopIcon*> enemiesIcons;
	std::list<Text*> squads;
	Text* text = nullptr;
	Chrono* timer = nullptr;

	iPoint firstIcon_pos = { 0,0 };
	iPoint icons_offset = { 0,0 };

};

#endif // _UI_NEXTWAVEWINDOW_H__