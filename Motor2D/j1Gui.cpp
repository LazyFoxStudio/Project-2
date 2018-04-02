#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Audio.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "j1Fonts.h"
#include "j1Input.h"
#include "j1Gui.h"
#include "Entity.h"
#include "UI_element.h"
#include "UI_Image.h"
#include "UI_Text.h"
#include "UI_Button.h"
#include "UI_Window.h"
#include "UI_LifeBar.h"
#include "j1Window.h"
#include "j1UIScene.h"
#include "UI_Chrono.h"
#include "UI_ProgressBar.h"
#include "j1EntityController.h"
#include "UI_IngameMenu.h"

j1Gui::j1Gui() : j1Module()
{
	name = "gui";
}

// Destructor
j1Gui::~j1Gui()
{}

// Called before render is available
bool j1Gui::Awake(pugi::xml_node& conf)
{
	LOG("Loading GUI atlas");
	bool ret = true;

	buttonFX = conf.child("buttonFX").attribute("source").as_string("");
	atlas_file_name = conf.child("atlas").attribute("file").as_string("");
	icon_atlas_file_name = conf.child("icon_atlas").attribute("file").as_string("");

	return ret;
}

// Called before the first frame
bool j1Gui::Start()
{
	atlas = App->tex->Load(atlas_file_name.c_str());
	icon_atlas = App->tex->Load(icon_atlas_file_name.c_str());

	button_click_fx = App->audio->LoadFx(buttonFX.c_str());

	pugi::xml_document doc;
	pugi::xml_node gameData;

	gameData = App->LoadFile(doc, "GameData.xml");

	LoadDB(gameData);

	return true;
}

// Update all guis
bool j1Gui::PreUpdate()
{
	bool ret = true;
	
	//SDL_SetTextureAlphaMod(atlas, alpha_value);

	UI_element* element = nullptr;

	//Get element to interact with
	if (draggingElement != nullptr)
		element = draggingElement;
	else
	{
		for (std::list<menu*>::reverse_iterator it_m = App->uiscene->menus.rbegin(); it_m != App->uiscene->menus.rend(); it_m++) //Go through menus
		{
			if ((*it_m) == nullptr) break;
			if ((*it_m)->active == false) continue;
			for (std::list<UI_element*>::iterator it_e = (*it_m)->elements.begin(); it_e != (*it_m)->elements.end(); it_e++) //Go through elements
			{
				if (checkMouseHovering((*it_e)))
					element = (*it_e);
				for (std::list<UI_element*>::iterator it_c = (*it_e)->childs.begin(); it_c != (*it_e)->childs.end(); it_c++)
					if (checkMouseHovering((*it_c)))
						element = (*it_c);
			}
		}
	}

	//Send events related to UI elements
	if (element != nullptr)
	{
		if (!element->hovering)
		{
			element->hovering = true;
			if (element->callback != nullptr)
				element->callback->OnUIEvent(element, MOUSE_ENTER);
		}
		else if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN)
		{
			clickedOnUI = true;
			if (element->callback != nullptr)
			{
				ret = element->callback->OnUIEvent(element, MOUSE_LEFT_CLICK);
			}
			if (element->element_type == BUTTON)
				App->audio->PlayFx(button_click_fx, 0);

			if (element->dragable)
			{
				element->Start_Drag();
				draggingElement = element;
			}
		}
		else if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP)
		{
			if (element->callback != nullptr)
			{
				element->callback->OnUIEvent(element, MOUSE_LEFT_RELEASE);
			}
			if (element->dragable)
			{
				element->End_Drag();
				draggingElement = nullptr;
			}
		}
		else if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_DOWN)
		{
			if (element->callback != nullptr)
				ret = element->callback->OnUIEvent(element, MOUSE_RIGHT_CLICK);
		}
		else if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_UP)
		{
			if (element->callback != nullptr)
				element->callback->OnUIEvent(element, MOUSE_RIGHT_RELEASE);
		}
	}

	return ret;
}

bool j1Gui::Update(float dt)
{
	if (App->input->GetKey(SDL_SCANCODE_F8) == KEY_DOWN)
		UI_Debug = !UI_Debug;

	return true;
}

// Called after all Updates
bool j1Gui::PostUpdate()
{
	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP)
		clickedOnUI = false;

	//Draw selection quads
	for (std::list<Entity*>::iterator it_e = App->entitycontroller->selected_entities.begin(); it_e != App->entitycontroller->selected_entities.end(); it_e++)
		App->render->DrawQuad((*it_e)->collider, Green, false);
	
	//Draw LifeBars
	for (std::list<LifeBar*>::iterator it_l = LifeBars.begin(); it_l != LifeBars.end(); it_l++)
	{
		(*it_l)->BlitElement();
	}
	//Draw elements of active menus
	for (std::list<menu*>::iterator it_m = App->uiscene->menus.begin(); it_m != App->uiscene->menus.end(); it_m++)
	{
		if ((*it_m) == nullptr) break;
		if (!(*it_m)->active) continue;
		for (std::list<UI_element*>::iterator it_e = (*it_m)->elements.begin(); it_e != (*it_m)->elements.end(); it_e++)
		{
			if ((*it_e)->moving)
			{
				(*it_e)->Mouse_Drag();
				(*it_e)->state = CLICKED;
			}

			if ((*it_e)->parent == nullptr) //If it has a parent, the parent will be responsible for drawing it
				(*it_e)->BlitElement();
		}
	}
	//Draw Debug
	if (UI_Debug)
		UIDebugDraw();
	
	return true;
}

// Called before quitting
bool j1Gui::CleanUp()
{
	LOG("Freeing GUI");

	if (atlas != nullptr)
	{
		App->tex->UnLoad(atlas);
		atlas = nullptr;
	}
	if (icon_atlas != nullptr)
	{
		App->tex->UnLoad(icon_atlas);
		icon_atlas = nullptr;
	}

	//Texts
	std::list<Text*>::iterator it_t;
	it_t = Texts.begin();
	while ((*it_t) != nullptr && it_t != Texts.end())
	{
		RELEASE((*it_t));
		it_t++;
	}
	Texts.clear();
	//Images
	std::list<Image*>::iterator it_i;
	it_i = Images.begin();
	while ((*it_i) != nullptr && it_i != Images.end())
	{
		RELEASE((*it_i));
		it_i++;
	}
	Images.clear();
	//Buttons
	std::list<Button*>::iterator it_b;
	it_b = Buttons.begin();
	while ((*it_b) != nullptr && it_b != Buttons.end())
	{
		RELEASE((*it_b));
		it_b++;
	}
	Buttons.clear();
	//Windows
	std::list<Window*>::iterator it_w;
	it_w = Windows.begin();
	while ((*it_w) != nullptr && it_w != Windows.end())
	{
		RELEASE((*it_w));
		it_w++;
	}
	Windows.clear();
	//Chronos
	std::list<Chrono*>::iterator it_c;
	it_c = Chronos.begin();
	while ((*it_c) != nullptr && it_c != Chronos.end())
	{
		RELEASE((*it_c));
		it_c++;
	}
	Chronos.clear();
	//ProgressBars
	std::list<ProgressBar*>::iterator it_p;
	it_p = ProgressBars.begin();
	while ((*it_p) != nullptr && it_p != ProgressBars.end())
	{
		RELEASE((*it_p));
		it_p++;
	}
	ProgressBars.clear();
	//LifeBars
	std::list<LifeBar*>::iterator it_l;
	it_l = LifeBars.begin();
	while ((*it_l) != nullptr && it_l != LifeBars.end())
	{
		RELEASE((*it_l));
		it_l++;
	}
	LifeBars.clear();

	return true;
}

bool j1Gui::checkMouseHovering(UI_element* element)
{
	int x, y;
	App->input->GetMousePosition(x, y);
	int scale = App->win->GetScale();
	bool ret = false;

	iPoint globalPos = element->calculateAbsolutePosition();
	if (x >= globalPos.x && x <= globalPos.x + element->section.w / scale && y >= globalPos.y && y <= globalPos.y + element->section.h / scale && element->interactive)
	{
		ret = true;
	}
	else if (element->hovering)
	{
		element->hovering = false;
		if (element->callback != nullptr)
			element->callback->OnUIEvent(element, MOUSE_LEAVE);
	}

	return ret;
}

void j1Gui::UIDebugDraw()
{
	for (std::list<menu*>::iterator it_m = App->uiscene->menus.begin(); it_m != App->uiscene->menus.end(); it_m++)
	{
		if ((*it_m) == nullptr) break;
		if (!(*it_m)->active) continue;
		for (std::list<UI_element*>::iterator it_e = (*it_m)->elements.begin(); it_e != (*it_m)->elements.end(); it_e++)
		{
			SDL_Rect box;
			int scale = App->win->GetScale();
			box.x = (*it_e)->calculateAbsolutePosition().x * scale;
			box.y = (*it_e)->calculateAbsolutePosition().y * scale;
			box.w = (*it_e)->section.w;
			box.h = (*it_e)->section.h;
			App->render->DrawQuad(box, Red, false, false);
		}
	}

}
// const getter for atlas
const SDL_Texture* j1Gui::GetAtlas() const
{
	return atlas;
}

const SDL_Texture* j1Gui::GetIconAtlas() const
{
	return icon_atlas;
}

UI_element* j1Gui::GetElement(int type, int id)
{
	UI_element* ret = nullptr;
	switch (type)
	{
	case TEXT:
		ret = (*std::next(Texts.begin(), id));
		break;
	case IMAGE:
		ret = (*std::next(Images.begin(), id));
		break;
	case BUTTON:
		ret = (*std::next(Buttons.begin(), id));
		break;
	case WINDOW:
		ret = (*std::next(Windows.begin(), id));
		break;
	case CHRONO:
		ret = (*std::next(Chronos.begin(), id));
		break;
	case PROGRESSBAR:
		ret = (*std::next(ProgressBars.begin(), id));
		break;
	}

	return ret;
}

void j1Gui::Load_UIElements(pugi::xml_node node, menu* menu, j1Module* callback, UI_element* parent)
{
	BROFILER_CATEGORY("Load UI elements", Profiler::Color::Chocolate);

	pugi::xml_node tmp;

	tmp = node.first_child();
	UI_element* element = nullptr;
	for (; tmp; tmp = tmp.next_sibling())
	{
		std::string type = tmp.name();
		if (type == "atlas_image")
			element = createImageFromAtlas(tmp, callback, tmp.attribute("icon_atlas").as_bool(false));
		else if (type == "text")
			element = createText(tmp, callback);
		else if (type == "timer")
			element = createTimer(tmp, callback);
		else if (type == "stopwatch")
			element = createStopWatch(tmp, callback);
		else if (type == "image")
			element = createImage(tmp, callback);
		else if (type == "button")
			element = createButton(tmp, callback);
		else if (type == "window")
			element = createWindow(tmp, callback);
		else if (type == "progressbar")
			element = createProgressBar(tmp, callback);
		else if (type == "ingamemenu")
			element = createIngameMenu(tmp, callback);

		element->setDragable(tmp.child("draggable").attribute("horizontal").as_bool(false), tmp.child("draggable").attribute("vertical").as_bool(false));
		element->interactive = tmp.child("interactive").attribute("value").as_bool(true);

		pugi::xml_node childs = tmp.child("childs");
		if(childs)
		{ 
			Load_UIElements(childs, nullptr, callback, element);
		}
		
		if (parent != nullptr)
		{
			parent->appendChild(element, tmp.attribute("center").as_bool());
		}

		if (menu != nullptr)
			menu->elements.push_back(element);
	}
}

Text* j1Gui::createText(pugi::xml_node node, j1Module* callback, bool saveIntoGUI)
{
	std::string text = node.attribute("text").as_string();
	if (node.attribute("counter").as_bool())
	{
		text = "Element: " + std::to_string(Texts.size());
	}
	int x = node.child("position").attribute("x").as_int();
	int y = node.child("position").attribute("y").as_int();
	int font_id = node.child("font").attribute("id").as_int();
	std::list<_TTF_Font*>::iterator font = std::next(App->font->fonts.begin(), font_id-1);
	SDL_Color color = { node.child("color").attribute("r").as_int(), node.child("color").attribute("g").as_int(), node.child("color").attribute("b").as_int(), node.child("color").attribute("a").as_int() };

	Text* ret = new Text(text, x, y, (*font), color, callback);

	if (saveIntoGUI)
		Texts.push_back(ret);

	return ret;
}

Image* j1Gui::createImage(pugi::xml_node node, j1Module* callback, bool saveIntoGUI)
{
	SDL_Texture* texture = App->tex->Load(node.attribute("path").as_string());
	uint tex_width, tex_height;
	App->tex->GetSize(texture, tex_width, tex_height);

	int x = node.child("position").attribute("x").as_int();
	int y = node.child("position").attribute("y").as_int();

	Image* ret = new Image(texture, x, y, {0, 0, (int)tex_width, (int)tex_height }, callback);

	if (saveIntoGUI)
		Images.push_back(ret);

	return ret;
}

Image* j1Gui::createImageFromAtlas(pugi::xml_node node, j1Module* callback, bool use_icon_atlas, bool saveIntoGUI)
{
	int x = node.child("position").attribute("x").as_int();
	int y = node.child("position").attribute("y").as_int();
	SDL_Rect section = { node.child("section").attribute("x").as_int(), node.child("section").attribute("y").as_int(), node.child("section").attribute("w").as_int(), node.child("section").attribute("h").as_int() };

	SDL_Texture* usingAtlas = (use_icon_atlas) ? icon_atlas : atlas;
	Image* ret = new Image(usingAtlas, x, y, section, callback);

	if (saveIntoGUI)
		Images.push_back(ret);

	return ret;
}

Window* j1Gui::createWindow(pugi::xml_node node, j1Module * callback, bool saveIntoGUI)
{
	SDL_Texture* texture = nullptr;
	if (node.attribute("path"))
		texture = App->tex->Load(node.attribute("path").as_string());
	else
		texture = atlas;

	int x = node.child("position").attribute("x").as_int();
	int y = node.child("position").attribute("y").as_int();
	SDL_Rect section = { node.child("section").attribute("x").as_int(), node.child("section").attribute("y").as_int(), node.child("section").attribute("w").as_int(), node.child("section").attribute("h").as_int() };

	Window* ret = new Window(texture, x, y, section, callback);

	if (saveIntoGUI)
		Windows.push_back(ret);

	return ret;
}

Button* j1Gui::createButton(pugi::xml_node node, j1Module* callback, bool saveIntoGUI)
{
	SDL_Texture* texture = nullptr;
	if (node.attribute("path"))
		texture = App->tex->Load(node.attribute("path").as_string());
	else
		texture = atlas;

	int x = node.child("position").attribute("x").as_int();
	int y = node.child("position").attribute("y").as_int();
	SDL_Rect standby = { node.child("standby").attribute("x").as_int(), node.child("standby").attribute("y").as_int(), node.child("standby").attribute("w").as_int(), node.child("standby").attribute("h").as_int() };
	SDL_Rect OnMouse = { node.child("OnMouse").attribute("x").as_int(), node.child("OnMouse").attribute("y").as_int(), node.child("OnMouse").attribute("w").as_int(), node.child("OnMouse").attribute("h").as_int() };
	SDL_Rect OnClick = { node.child("OnClick").attribute("x").as_int(), node.child("OnClick").attribute("y").as_int(), node.child("OnClick").attribute("w").as_int(), node.child("OnClick").attribute("h").as_int() };

	Button* ret = new Button(x, y, texture, standby, OnMouse, OnClick, callback);

	if (saveIntoGUI)
		Buttons.push_back(ret);

	return ret;
}

Chrono * j1Gui::createTimer(pugi::xml_node node, j1Module * callback, bool saveIntoGUI)
{
	int x = node.child("position").attribute("x").as_int();
	int y = node.child("position").attribute("y").as_int();

	int font_id = node.child("font").attribute("id").as_int();
	std::list<_TTF_Font*>::iterator font = std::next(App->font->fonts.begin(), font_id - 1);
	SDL_Color color = { node.child("color").attribute("r").as_int(), node.child("color").attribute("g").as_int(), node.child("color").attribute("b").as_int(), node.child("color").attribute("a").as_int() };
	
	Chrono* ret = new Chrono(x, y, TIMER, (*font), color, callback);
	
	ret->setStartValue(node.attribute("initial_value").as_int());
	if (saveIntoGUI)
		Chronos.push_back(ret);

	return ret;
}

Chrono * j1Gui::createStopWatch(pugi::xml_node node, j1Module * callback, bool saveIntoGUI)
{
	int x = node.child("position").attribute("x").as_int();
	int y = node.child("position").attribute("y").as_int();
	
	int font_id = node.child("font").attribute("id").as_int();
	std::list<_TTF_Font*>::iterator font = std::next(App->font->fonts.begin(), font_id - 1);
	SDL_Color color = { node.child("color").attribute("r").as_int(), node.child("color").attribute("g").as_int(), node.child("color").attribute("b").as_int(), node.child("color").attribute("a").as_int() };

	Chrono* ret = new Chrono(x, y, STOPWATCH, (*font), color, callback);

	if (saveIntoGUI)
		Chronos.push_back(ret);

	return ret;
}

ProgressBar* j1Gui::createProgressBar(pugi::xml_node node, j1Module* callback, bool saveIntoGUI)
{
	SDL_Texture* texture = nullptr;
	if (node.attribute("path"))
		texture = App->tex->Load(node.attribute("path").as_string());
	else
		texture = atlas;

	int x = node.child("position").attribute("x").as_int();
	int y = node.child("position").attribute("y").as_int();

	SDL_Rect empty = { node.child("empty").attribute("x").as_int(), node.child("empty").attribute("y").as_int(), node.child("empty").attribute("w").as_int(), node.child("empty").attribute("h").as_int() };
	SDL_Rect full = { node.child("full").attribute("x").as_int(), node.child("full").attribute("y").as_int(), node.child("full").attribute("w").as_int(), node.child("full").attribute("h").as_int() };
	SDL_Rect head = { node.child("head").attribute("x").as_int(0), node.child("head").attribute("y").as_int(0), node.child("head").attribute("w").as_int(0), node.child("head").attribute("h").as_int(0) };

	float max_value = node.attribute("max_value").as_float(0);

	ProgressBar* ret = new ProgressBar(x, y, texture, empty, full, head, max_value, callback);

	if (saveIntoGUI)
		ProgressBars.push_back(ret);

	return ret;
}

IngameMenu* j1Gui::createIngameMenu(pugi::xml_node node, j1Module * callback)
{
	SDL_Texture* texture = nullptr;
	if (node.attribute("path"))
		texture = App->tex->Load(node.attribute("path").as_string());
	else
		texture = atlas;

	int x = node.child("position").attribute("x").as_int();
	int y = node.child("position").attribute("y").as_int();
	SDL_Rect section = { node.child("section").attribute("x").as_int(), node.child("section").attribute("y").as_int(), node.child("section").attribute("w").as_int(), node.child("section").attribute("h").as_int() };

	int minimap_posX = node.child("minimap").attribute("x").as_int();
	int minimap_posY = node.child("minimap").attribute("y").as_int();

	int firstIcon_posX = node.child("icons").attribute("x").as_int();
	int firstIcon_posY = node.child("icons").attribute("y").as_int();
	int icons_offsetX = node.child("icons").attribute("offsetX").as_int();
	int icons_offsetY = node.child("icons").attribute("offsetY").as_int();

	int lifeBars_offsetX = node.child("lifeBars").attribute("offsetX").as_int();
	int lifeBars_offsetY = node.child("lifeBars").attribute("offsetY").as_int();

	int stats_posX = node.child("stats").attribute("x").as_int();
	int stats_posY = node.child("stats").attribute("y").as_int();

	IngameMenu* ret = new IngameMenu(texture, icon_atlas, x, y, section, minimap_posX, minimap_posY, firstIcon_posX, firstIcon_posY, icons_offsetX, icons_offsetY, lifeBars_offsetX, lifeBars_offsetY, stats_posX, stats_posY, callback);

	ret->createActionButtons(node.child("buttons"));

	inGameMenu = ret;

	return ret;
}

void j1Gui::createLifeBar(Entity* entity)
{
	LifeBar* ret = new LifeBar(entity, atlas);

	LifeBars.push_back(ret);
}

void j1Gui::LoadDB(pugi::xml_node node)
{
	pugi::xml_node lifebar;
	pugi::xml_node rect;

	for (lifebar = node.child("LifeBars").child("in-game").first_child(); lifebar; lifebar = lifebar.next_sibling())
	{
		for (rect = lifebar.first_child(); rect; rect = rect.next_sibling())
		{
			SDL_Rect section = { rect.attribute("x").as_int(0), rect.attribute("y").as_int(0) , rect.attribute("w").as_int(0) , rect.attribute("h").as_int(0) };
			std::string tag = rect.attribute("tag").as_string();

			LifeBarRect.insert(std::pair<std::string, SDL_Rect>(tag, section));
		}
	}
	for (lifebar = node.child("LifeBars").child("menu").first_child(); lifebar; lifebar = lifebar.next_sibling())
	{
		for (rect = lifebar.first_child(); rect; rect = rect.next_sibling())
		{
			SDL_Rect section = { rect.attribute("x").as_int(0), rect.attribute("y").as_int(0) , rect.attribute("w").as_int(0) , rect.attribute("h").as_int(0) };
			std::string tag = rect.attribute("tag").as_string();

			LifeBarRect.insert(std::pair<std::string, SDL_Rect>(tag, section));
		}
	}
}

void j1Gui::AddIconData(unitType type, pugi::xml_node node)
{
	SDL_Rect rect = { node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() };
	unitIconRect.insert(std::pair<unitType, SDL_Rect>(type, rect));
}

void j1Gui::AddIconData(buildingType type, pugi::xml_node node)
{
	SDL_Rect rect = { node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() };
	buildingIconRect.insert(std::pair<buildingType, SDL_Rect>(type, rect));
}

void j1Gui::AddIconData(resourceType type, pugi::xml_node node)
{
	SDL_Rect rect = { node.attribute("x").as_int(), node.attribute("y").as_int(), node.attribute("w").as_int(), node.attribute("h").as_int() };
	resourceIconRect.insert(std::pair<resourceType, SDL_Rect>(type, rect));
}

SDL_Rect j1Gui::GetIconRect(Entity* entity)
{
	switch (entity->entity_type)
	{
	case UNIT:
		return unitIconRect.at(((Unit*)entity)->type);
		break;
	case BUILDING:
		return buildingIconRect.at(((Building*)entity)->type);
		break;
	case NATURE:
		return resourceIconRect.at(((Nature*)entity)->type);
		break;
	default:
		return { 0,0,0,0 };
		break;
	}
}

SDL_Rect j1Gui::GetLifeBarRect(std::string tag)
{
	return LifeBarRect.at(tag);
}

void j1Gui::newSelectionDone()
{
	if (inGameMenu != nullptr)
		inGameMenu->updateInfo();
}
