#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Audio.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "j1Fonts.h"
#include "j1Input.h"
#include "j1Gui.h"
#include "UI_element.h"
#include "UI_Image.h"
#include "UI_Text.h"
#include "UI_Button.h"
#include "UI_Window.h"
#include "j1Window.h"
#include "j1UIScene.h"
#include "UI_Chrono.h"
#include "UI_ProgressBar.h"


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

	return ret;
}

// Called before the first frame
bool j1Gui::Start()
{
	atlas = App->tex->Load(atlas_file_name.c_str());

	button_click_fx = App->audio->LoadFx(buttonFX.c_str());

	return true;
}

// Update all guis
bool j1Gui::PreUpdate()
{
	bool ret = true;
	
	//SDL_SetTextureAlphaMod(atlas, alpha_value);

	int x, y;
	App->input->GetMousePosition(x, y);
	int scale = App->win->GetScale();
	UI_element* element = nullptr;

	//Get element to interact with
	if (draggingElement != nullptr)
		element = draggingElement;
	else
	{
		for (std::list<menu*>::reverse_iterator it_m = App->uiscene->menus.rbegin(); it_m != App->uiscene->menus.rend(); it_m++)
		{
			if ((*it_m) == nullptr) break;
			if ((*it_m)->active == false) continue;
			for (std::list<UI_element*>::reverse_iterator it_e = (*it_m)->elements.rbegin()--; it_e != (*it_m)->elements.rend(); it_e++)
			{
				iPoint globalPos = (*it_e)->calculateAbsolutePosition();
				if (x > globalPos.x && x < globalPos.x + (*it_e)->section.w / scale && y > globalPos.y && y < globalPos.y + (*it_e)->section.h / scale && element == nullptr && (*it_e)->interactive)
				{
					element = (*it_e);
				}
				else if ((*it_e)->hovering)
				{
					(*it_e)->hovering = false;
					if ((*it_e)->callback != nullptr)
						(*it_e)->callback->OnUIEvent((*it_e), MOUSE_LEAVE);
				}
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
	if (UI_Debug)
		UIDebugDraw();
	
	return true;
}

// Called before quitting
bool j1Gui::CleanUp()
{
	LOG("Freeing GUI");

	std::list<UI_element*>::iterator it;
	it = UI_elements.begin();
	while (it != UI_elements.end())
	{
		RELEASE((*it));
		it++;
	}
	UI_elements.clear();

	return true;
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

void j1Gui::Load_UIElements(pugi::xml_node node, menu* menu, j1Module* callback)
{
	BROFILER_CATEGORY("Load UI elements", Profiler::Color::Chocolate);

	pugi::xml_node tmp;

	tmp = node.child("atlas_image");
	if (tmp)
	{
		UI_element* element = createImageFromAtlas(tmp, callback);
		menu->elements.push_back(element);
		while (tmp = tmp.next_sibling("atlas_image"))
		{
			element = createImageFromAtlas(tmp, callback);
			menu->elements.push_back(element);
		}
	}

}

Text* j1Gui::createText(char* text, int x, int y, _TTF_Font* font, SDL_Color color, j1Module* callback)
{
	Text* ret = new Text(text, x, y, font, color, callback);
	ret->interactive = false;
	UI_elements.push_back(ret);

	return ret;
}

Image* j1Gui::createImage(int x, int y, SDL_Texture* texture, j1Module* callback)
{
	uint tex_width, tex_height;
	App->tex->GetSize(texture, tex_width, tex_height);
	Image* ret = new Image(texture, x, y, {0, 0, (int)tex_width, (int)tex_height }, callback);
	UI_elements.push_back(ret);

	return ret;
}

Image* j1Gui::createImageFromAtlas(pugi::xml_node node, j1Module* callback)
{
	int x = node.child("position").attribute("x").as_int();
	int y = node.child("position").attribute("y").as_int();
	SDL_Rect section = { node.child("section").attribute("x").as_int(), node.child("section").attribute("y").as_int(), node.child("section").attribute("w").as_int(), node.child("section").attribute("h").as_int() };

	Image* ret = new Image(atlas, x, y, section, callback);
	ret->setDragable(node.child("draggable").attribute("horizontal").as_bool(), node.child("draggable").attribute("vertical").as_bool());
	ret->interactive = node.child("interactive").attribute("value").as_bool();
	UI_elements.push_back(ret);

	return ret;
}

Window* j1Gui::createWindow(int x, int y, SDL_Texture * texture, SDL_Rect section, j1Module * callback)
{
	SDL_Texture* usingTexture = (texture) ? texture : atlas;

	Window* ret = new Window(usingTexture, x, y, section, callback);
	UI_elements.push_back(ret);

	return ret;
}

Button* j1Gui::createButton(int x, int y, SDL_Texture* texture, SDL_Rect standby, SDL_Rect OnMouse, SDL_Rect OnClick, j1Module* callback)
{
	SDL_Texture* usingTexture = (texture) ? texture : atlas;

	Button* ret = new Button(x, y, usingTexture, standby, OnMouse, OnClick, callback);
	UI_elements.push_back(ret);

	return ret;
}

Chrono * j1Gui::createTimer(int x, int y, int initial_value, _TTF_Font * font, SDL_Color color, j1Module * callback)
{
	Chrono* ret = new Chrono(x, y, TIMER, font, color, callback);
	ret->setStartValue(initial_value);
	ret->interactive = false;
	UI_elements.push_back(ret);

	return ret;
}

Chrono * j1Gui::createStopWatch(int x, int y, _TTF_Font * font, SDL_Color color, j1Module * callback)
{
	Chrono* ret = new Chrono(x, y, STOPWATCH, font, color, callback);
	ret->interactive = false;
	UI_elements.push_back(ret);

	return ret;
}

ProgressBar* j1Gui::createProgressBar(int x, int y, SDL_Texture* texture, SDL_Rect empty, SDL_Rect full, Image* head, j1Module* callback)
{
	SDL_Texture* usingTexture = (texture) ? texture : atlas;

	ProgressBar* ret = new ProgressBar(x, y, usingTexture, empty, full, head, callback);
	UI_elements.push_back(ret);

	return ret;
}