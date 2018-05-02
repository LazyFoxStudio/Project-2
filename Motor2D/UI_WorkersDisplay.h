#ifndef __UI_WORKERSDISPLAY_H__
#define __UI_WORKERSDISPLAY_H__

#include "UI_element.h"

#define WORKERS_DISPLAY_OFFSET {0, -65}

class Button;
class Text;
class Image;
class Building;

class WorkersDisplay : public UI_element
{
public:
	WorkersDisplay(SDL_Rect icon, Button* increase, Button* decrease, Building* building);
	WorkersDisplay(WorkersDisplay* copy, Building* building);
	~WorkersDisplay();

	void BlitElement();

	UI_element* getMouseHoveringElement();

public:

	Image* icon = nullptr;
	Text* workers = nullptr;
	Button* increase = nullptr;
	Button* decrease = nullptr;

	Building* building = nullptr;
};

#endif //__UI_WORKERSDISPLAY_H__