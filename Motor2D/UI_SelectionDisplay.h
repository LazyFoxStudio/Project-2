#ifndef __UI_SELECTIONDISPLAY_H__
#define __UI_SELECTIONDISPLAY_H__

#include "UI_element.h"

#define FIRST_ICON_POSITION {376, 849}
#define TROOP_ICON_OFFSET {355, 67}
#define LIFEBARS_OFFSET {82, 6}
#define SQUAD_ICON_OFFSET 26

class Image;
class LifeBar;
class Entity;
class Squad;
class Text;

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

	bool active = true;
	std::vector<TroopIcon*> troopIcons;
};

class SelectionDisplay : public UI_element
{
public:
	SelectionDisplay();
	~SelectionDisplay();

	void newSelection();
	void OrderDisplay();
	void cleanLists();
	void deleteDisplay(Entity* entity);

	void BlitElement();
	void setAdditionalSquads();

public:

	std::list<TroopDisplay*> troops; //Being created/destroyed
	std::list<SquadDisplay*> squads; //Being created/destroyed

	Text* moreSquads = nullptr;
	bool severalSquads = false;
};

#endif //__UI_SELECTIONDISPLAY_H__