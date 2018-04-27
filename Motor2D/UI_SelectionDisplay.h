#ifndef __UI_SELECTIONDISPLAY_H__
#define __UI_SELECTIONDISPLAY_H__

#include "UI_element.h"

#define FIRST_ICON_POSITION {296, 816}
#define TROOP_ICON_OFFSET {398, 78}
#define LIFEBARS_OFFSET {82, 6}
#define SQUAD_ICON_OFFSET 26

class Image;
class LifeBar;
class Entity;
class Squad;

struct TroopIcon
{
	TroopIcon(Entity* entity, int x, int y);
	~TroopIcon()
	{
		RELEASE(image);
	}

	void updateColor();

	Image* image = nullptr;
	const Entity* entity = nullptr;
};

struct TroopDisplay
{
	TroopDisplay(Entity* entity, int x, int y);
	~TroopDisplay()
	{
		RELEASE(icon);
		RELEASE(lifeBar);
	}

	void Draw();

	TroopIcon* icon = nullptr;
	LifeBar* lifeBar = nullptr;
};

struct SquadDisplay
{
	SquadDisplay(Squad* squad, int x, int y);
	~SquadDisplay()
	{
		for (int i = 0; i < troopIcons.size(); i++)
		{
			RELEASE(troopIcons[i]);
		}
		troopIcons.clear();
	}

	void Draw();

	std::vector<TroopIcon*> troopIcons;
};

class SelectionDisplay : public UI_element
{
public:
	SelectionDisplay(): UI_element(0,0,element_type::WINDOW, {0,0,0,0}, nullptr)
	{}
	~SelectionDisplay();

	void newSelection();
	void OrderDisplay();
	void cleanLists();
	void deleteDisplay(Entity* entity);

	void BlitElement(bool use_camera = false);

public:

	std::list<TroopDisplay*> troops; //Being created/destroyed
	std::list<SquadDisplay*> squads; //Being created/destroyed

	bool severalSquads = false;
};

#endif //__UI_SELECTIONDISPLAY_H__