#ifndef __UI_FARMWORKERSMANAGER_H__
#define __UI_FARMWORKERSMANAGER_H__

#include "UI_element.h"

#define ICONS_OFFSET {100, 80}

class ProgressBar;
class j1Timer;
class Text;
class Image;
struct worker;

enum worker_state
{
	UNASSIGNED,
	ASSIGNED,
	PRODUCING,
	WAITING
};

struct workerIcon
{
	workerIcon(worker* worker);
	~workerIcon()
	{
		RELEASE(icon);
		RELEASE(producing);
		RELEASE(waiting);
	}

	Image* icon = nullptr;
	Text* producing = nullptr;
	Text* waiting = nullptr;
	j1Timer timer;
	worker_state state = UNASSIGNED;
	worker* Worker = nullptr;
};

class FarmWorkersManager : public UI_element
{
public:
	FarmWorkersManager();
	~FarmWorkersManager();

public:

	std::vector<workerIcon*> icon;
};

#endif //__UI_FARMWORKERSMANAGER_H__