#ifndef __j1GUI_H__
#define __j1GUI_H__

#include "j1Module.h"
#include "p2Point.h"
#include "SDL/include/SDL_rect.h"
#include "Unit.h"
#include "Building.h"
#include "Hero.h"
#include <map>
#include <string>
#include <list>

#define DEFAULT_RESOLUTION {1680, 1050}

struct _TTF_Font;
struct SDL_Texture;
class UI_element;
class Text;
class Image;
class Button;
class Window;
class Chrono;
class ProgressBar;
class LifeBar;
class IngameMenu;
class menu;
class Entity;
class CostDisplay;
class WarningMessages;
class NextWaveWindow;
class WorkersDisplay;
class Building;
class CooldownsDisplay;
class TroopCreationQueue;
class FarmWorkersManager;
class Minimap;
class Slider;

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

	bool checkMouseHovering(UI_element* element);

	void UIDebugDraw();
	// Gui creation functions

	//Type:
	//TEXT - 0
	//IMAGE - 1
	//BUTON - 2
	//WINDOW - 3
	//CHRONO - 4
	//PROGRESSBAR - 5
	UI_element* GetElement(int type, int id);

	void Load_UIElements(pugi::xml_node node, menu* menu, j1Module* callback = nullptr, UI_element* parent = nullptr);
	Text* createText(pugi::xml_node node, j1Module* callback = nullptr, bool saveIntoGUI = true);
	Chrono* createTimer(pugi::xml_node node, j1Module* callback = nullptr, bool saveIntoGUI = true);
	Chrono* createStopWatch(pugi::xml_node node, j1Module* callback = nullptr, bool saveIntoGUI = true);
	Image* createImage(pugi::xml_node node, j1Module* callback = nullptr, bool saveIntoGUI = true);
	Image* createImageFromAtlas(pugi::xml_node node, j1Module* callback = nullptr, bool use_icon_atlas = false, bool saveIntoGUI = true);
	//NULL texture to use atlas
	Button* createButton(pugi::xml_node node, j1Module* callback = nullptr, bool saveIntoGUI = true);
	Button* createSwitch(pugi::xml_node node, j1Module* callback = nullptr, bool saveIntoGUI = true);
	Window* createWindow(pugi::xml_node node, j1Module* callback = nullptr, bool saveIntoGUI = true);
	ProgressBar* createProgressBar(pugi::xml_node node, j1Module* callback = nullptr, bool saveIntoGUI = true);
	IngameMenu* createIngameMenu(pugi::xml_node node, j1Module* callback = nullptr);
	NextWaveWindow* createNextWaveWindow(pugi::xml_node node, j1Module* callback = nullptr);
	WorkersDisplay* createWorkersDisplay(Building* building);
	TroopCreationQueue* createTroopCreationQueue(Building* building);
	Slider* j1Gui::createSlider(pugi::xml_node node, j1Module* callback = nullptr, bool saveIntoGUI = true);


	//minimap_
	void createMinimap(pugi::xml_node node, j1Module* callback = nullptr);

	void createLifeBar(Entity* entity);
	void deleteLifeBar(Entity* entity, std::list<LifeBar*>& list);
	void entityDeleted(Entity* entity);
	CostDisplay* createCostDisplay(std::string name, int wood_cost = 0, int gold_cost = 0, int oil_cost = 0, int workers_cost = 0,  uint upgradetype = 0, uint upgradelvl = 3);

	void deleteElement(UI_element* element);

	void createPopUpInfo(UI_element* element, std::string info);

	void LoadLifeBarsDB(pugi::xml_node node);
	void LoadActionButtonsDB(pugi::xml_node node);
	void LoadWorkersDisplayDB(pugi::xml_node node);
	void LoadFonts(pugi::xml_node node);

	void AddIconDataUnit(Type type, pugi::xml_node node);
	/*void AddIconData(heroType type, pugi::xml_node node);*/
	void AddIconDataBuilding(Type type, pugi::xml_node node);
	SDL_Rect GetIconRect(Entity* entity);
	//Hardcoded
	SDL_Rect GetUnitRect(Type type);
	SDL_Rect GetLifeBarRect(std::string tag);
	Button* GetActionButton(uint id);
	std::list<Button*> activateActionButtons(std::vector<uint> buttons);
	bool checkActionButtonsHotkeys();

	void newSelectionDone();
	void newWave();

	void moveElementToMouse(UI_element* element);

	void assignActionButtonHotkey(uint id, SDL_Scancode newHotkey);
	Button* searchButtonbyHotkey(SDL_Scancode hotkey) const;
	Button* getButtonbyId(uint id) const;
	uint getIDbyButton(Button* button) const;

	float getSliderProgress(int action) const;

public:

	SDL_Texture* atlas = nullptr;
	SDL_Texture* icon_atlas = nullptr;

	float w_stretch = 1.0f;
	float h_stretch = 1.0f;

	uint popUp_wait_time = 0;
	bool UI_Debug = false;
	int alpha_value = 255;
	bool leftClickedOnUI = false;
	bool rightClickedOnUI = false;
	j1PerfTimer hovering_element;
	UI_element* current_hovering_element = nullptr;
	WarningMessages* warningMessages = nullptr;
	Chrono* Chronos = nullptr;
	NextWaveWindow* nextWaveWindow = nullptr;
	CooldownsDisplay* cooldownsDisplay = nullptr;
	FarmWorkersManager* workersManager = nullptr;
	Minimap* minimap = nullptr;

private:

	std::string atlas_file_name = "";
	std::string icon_atlas_file_name = "";
	std::string	buttonFX = "";

	std::list<Image*> Images;
	std::list<Text*> Texts;
	std::list<Button*> Buttons;
	std::list<ProgressBar*> ProgressBars;
	std::list<LifeBar*> LifeBars;
	std::list<Window*> Windows;

	IngameMenu* inGameMenu = nullptr;
	UI_element* draggingElement = nullptr;
	WorkersDisplay* workersDisplayBase = nullptr;

	std::map<Type, SDL_Rect> unitIconRect;
	/*std::map<heroType, SDL_Rect> heroIconRect;*/
	std::map<Type, SDL_Rect> buildingIconRect;
	std::map<std::string, SDL_Rect> LifeBarRect;
	std::map<uint, Button*> actionButtons;
};

#endif // __j1GUI_H__