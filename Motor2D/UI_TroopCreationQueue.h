#ifndef __UI_TROOPCREATIONQUEUE_H__
#define __UI_TROOPCREATIONQUEUE_H__

#include "UI_element.h"

#define ICONS_OFFSET {100, 80}

class TroopIcon;
class ProgressBar;
class j1Timer;
class Text;

struct troopCreation
{
	ProgressBar* progress = nullptr;
	TroopIcon* icon = nullptr;
	j1Timer timer;
};

class TroopCreationQueue : public UI_element
{
public:
	TroopCreationQueue();
	~TroopCreationQueue();

	void BlitElement();
	void pushTroop(Type type);

public:

	Text* text = nullptr;
	std::list<troopCreation*> icons;
};

#endif //__UI_TROOPCREATIONQUEUE_H__