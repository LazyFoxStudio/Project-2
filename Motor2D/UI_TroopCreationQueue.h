#ifndef __UI_TROOPCREATIONQUEUE_H__
#define __UI_TROOPCREATIONQUEUE_H__

#include "UI_element.h"

#define ICONS_OFFSET {95, 80}

class TroopIcon;
class ProgressBar;
class j1Timer;
class Text;
class Building;

struct troopCreation
{
	ProgressBar* progress = nullptr;
	TroopIcon* icon = nullptr;
	j1Timer timer;
};

class TroopCreationQueue : public UI_element
{
public:
	TroopCreationQueue(Building* building);
	~TroopCreationQueue();

	void BlitElement();
	void pushTroop(Type type);

	UI_element* getMouseHoveringElement();

public:

	Text* text = nullptr;
	std::list<troopCreation*> icons;
	Building* building = nullptr;
};

#endif //__UI_TROOPCREATIONQUEUE_H__