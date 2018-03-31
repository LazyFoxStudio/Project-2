#ifndef __j1INTROSCENE_H__
#define __j1INTROSCENE_H__

#include "j1Module.h"
#include <list>

enum menu_id
{
	START_MENU,
	SETTINGS_MENU,
	INGAME_MENU,
	PAUSE_MENU,
	CREDITS_MENU,
	CONTROLS_MENU
};

struct menu
{
	menu(menu_id id) : id(id)
	{}

	std::list<UI_element*> elements;
	menu_id id;
	bool active = false;
};

class j1UIScene : public j1Module
{
public:
	j1UIScene();
	virtual ~j1UIScene();

	// Called before the first frame
	bool Start();


	// Called each loop iteration
	bool Update(float dt);

	void LoadFonts(pugi::xml_node node);
	void LoadUI(pugi::xml_node node);

	bool OnUIEvent(UI_element* element, event_type event_type);

	// Called before quitting
	bool CleanUp();

public:

	std::list<menu*> menus;
	int x, y;
};

#endif
