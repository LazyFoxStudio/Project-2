#ifndef __UI_FARMWORKERSMANAGER_H__
#define __UI_FARMWORKERSMANAGER_H__

#include "UI_element.h"

#define ICONS_OFFSET {100, 80}

class ProgressBar;
class j1Timer;
class Text;
class Image;
struct worker;
class Building;

enum worker_state
{
	UNASSIGNED,
	WORKING,
	GENERATING,
	WAITING
};

struct workerIcon
{
	workerIcon(int x, int y);
	~workerIcon()
	{
		RELEASE(icon);
		RELEASE(dark_icon);
		RELEASE(generating);
		RELEASE(waiting);
		RELEASE(progress);
		RELEASE(unassigned);
		RELEASE(working);
	}

	void Draw(worker_state state);

	Image* icon = nullptr;
	Image* dark_icon = nullptr;
	Text* unassigned = nullptr;
	Text* working = nullptr;
	Text* generating = nullptr;
	Text* waiting = nullptr;
	ProgressBar* progress = nullptr;
};

class FarmWorkersManager : public UI_element
{
public:
	FarmWorkersManager();
	~FarmWorkersManager();

	void BlitElement();

public:

	std::vector<workerIcon*> icons;
};

#endif //__UI_FARMWORKERSMANAGER_H__