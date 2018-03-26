#ifndef __j1GUI_H__
#define __j1GUI_H__

#include "j1Module.h"
#include "p2Point.h"
#include "SDL/include/SDL_rect.h"
#include "Unit.h"
#include "Building.h"
#include <map>
#include <string>
#include <list>

struct _TTF_Font;
struct SDL_Texture;
class UI_element;
class Text;
class Image;
class Button;
class Window;
class Chrono;
class ProgressBar;
class menu;

enum event_type
{
	MOUSE_ENTER,
	MOUSE_LEAVE,
	MOUSE_RIGHT_CLICK,
	MOUSE_RIGHT_RELEASE,
	MOUSE_LEFT_CLICK,
	MOUSE_LEFT_RELEASE,
	TIMER_ZERO,
	STOPWATCH_ALARM
};

// ---------------------------------------------------
class j1Gui : public j1Module
{
public:

	j1Gui();

	// Destructor
	virtual ~j1Gui();

	// Called when before render is available
	bool Awake(pugi::xml_node&);

	// Call before first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	bool Update(float dt);

	// Called after all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	void UIDebugDraw();
	// Gui creation functions
	const SDL_Texture* GetAtlas() const;
	UI_element* GetElement(int id);

	void Load_UIElements(pugi::xml_node node, menu* menu, j1Module* callback = nullptr, UI_element* parent = nullptr);
	Text* createText(pugi::xml_node node, j1Module* callback = nullptr);
	Chrono* createTimer(pugi::xml_node node, j1Module* callback = nullptr);
	Chrono* createStopWatch(pugi::xml_node node, j1Module* callback = nullptr);
	Image* createImage(pugi::xml_node node, j1Module* callback = nullptr);
	Image* createImageFromAtlas(pugi::xml_node node, j1Module* callback = nullptr);
	//NULL texture to use atlas
	Button* createButton(pugi::xml_node node, j1Module* callback = nullptr);
	Window* createWindow(pugi::xml_node node, j1Module* callback = nullptr);
	ProgressBar* createProgressBar(pugi::xml_node node, j1Module* callback = nullptr);

	void AddIconData(unitType type, pugi::xml_node node);
	void AddIconData(buildingType type, pugi::xml_node node);
	SDL_Rect GetIconRect(unitType type);
	SDL_Rect GetIconRect(buildingType type);

public:
	bool UI_Debug = false;
	int alpha_value = 255;
	uint button_click_fx = 0;

private:

	SDL_Texture* atlas;
	std::string atlas_file_name;
	std::string	buttonFX;
	std::list<UI_element*> UI_elements;
	UI_element* draggingElement = nullptr;
	std::map<unitType, SDL_Rect> unitIconRect;
	std::map<buildingType, SDL_Rect> buildingIconRect;
};

#endif // __j1GUI_H__