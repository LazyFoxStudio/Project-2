#ifndef __UI_UNLOCKDPLAY_H__
#define __UI_UNLOCKDPLAY_H__

#include "UI_element.h"
#include "j1Timer.h"

#define DEFAULT_UNLOCK_DISPLAY_TIME 2000

class Text;
class Image;

struct Unlockment
{
	~Unlockment();

	Text* title = nullptr;
	Image* icon = nullptr;
	Text* name = nullptr;

	j1Timer timer;
};

class UnlockDisplay : public UI_element
{
public:
	UnlockDisplay();
	~UnlockDisplay();

	void BlitElement();

	void unlockedAbility(SDL_Rect icon, std::string name);
	void unlockedBuilding(SDL_Rect icon, std::string name);

public:

	std::list<Unlockment*> unlockments;
};

#endif //__UI_UNLOCKDPLAY_H__