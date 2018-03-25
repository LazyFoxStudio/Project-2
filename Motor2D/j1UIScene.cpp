#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Input.h"
#include "j1Textures.h"
#include "j1Audio.h"
#include "j1Render.h"
#include "j1Scene.h"
#include "j1SceneSwitch.h"
#include "j1UIScene.h"
#include "j1Gui.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "UI_Element.h"
#include "UI_Chrono.h"
#include "UI_Text.h"
#include "j1Fonts.h"

j1UIScene::j1UIScene() { name = "introscene";}

j1UIScene::~j1UIScene() {}


bool j1UIScene::Start()
{
	pugi::xml_document	Gui_config_file;
	pugi::xml_node		guiconfig;

	guiconfig = App->LoadFile(Gui_config_file, "Gui_config.xml");

	// App->audio->PlayMusic("MainTheme.ogg");

	LoadUI(guiconfig);

	for (std::list<menu*>::iterator it_m = App->uiscene->menus.begin(); it_m != App->uiscene->menus.end(); it_m++)
	{
		if ((*it_m) == nullptr) break;
		if (!(*it_m)->active) continue;
		for (std::list<UI_element*>::iterator it_e = (*it_m)->elements.begin(); it_e != (*it_m)->elements.end(); it_e++)
		{
			if ((*it_e)->element_type == TEXT)
			{
				Text* text = (Text*)(*it_e);
				text->convertIntoCounter(&text->parent->localPosition.x);
			}
		}
	}

	return true;
}


bool j1UIScene::Update(float dt)
{

	return true;
}

void j1UIScene::LoadFonts(pugi::xml_node node)
{
	pugi::xml_node font;
	for (font = node.child("font"); font; font = font.next_sibling("font"))
	{
		App->font->Load(font.attribute("path").as_string(), font.attribute("size").as_int());
	}
}

void j1UIScene::LoadUI(pugi::xml_node node)
{
	LoadFonts(node);
	pugi::xml_node menuconfig;
	for (menuconfig = node.child("menu"); menuconfig; menuconfig = menuconfig.next_sibling("menu"))
	{
		menu* newMenu = nullptr;
		switch (menuconfig.attribute("type").as_int())
		{
		case 2:
			newMenu = new menu(INGAME_MENU);
			break;
		}
		App->gui->Load_UIElements(menuconfig, newMenu, this);
		newMenu->active = menuconfig.attribute("active").as_bool();
		menus.push_back(newMenu);
	}
}


bool j1UIScene::CleanUp()
{
	return true;
}

bool j1UIScene::OnUIEvent(UI_element* element, event_type event_type)
{
	bool ret = true;

	if (event_type == MOUSE_ENTER)
	{
		element->state = MOUSEOVER;

	}
	else if (event_type == MOUSE_LEAVE)
	{
		element->state = STANDBY;

	}
	else if (event_type == MOUSE_LEFT_CLICK)
	{
		element->state = CLICKED;

		switch (element->function)
		{
		case NEW_GAME:
			break;
		case CONTINUE:
			break;
		case SETTINGS:
			break;
		case CREDITS:
			break;
		case EXIT:
			ret = false;
			break;
		case PAUSE:
			break;
		case APPLY:
			break;
		case CANCEL:
			break;
		case BACK:
			break;
		case RESTORE:
			break;
		case HOME:
			break;
		case WEB:
			//App->RequestBrowser("https://adria-f.github.io/Game-Development/");
			break;
		}
	}
	else if (event_type == MOUSE_LEFT_RELEASE)
	{
		if (element->state == CLICKED)
			element->state = MOUSEOVER;
	}
	else if (event_type == MOUSE_RIGHT_CLICK)
	{
	}
	else if (event_type == MOUSE_RIGHT_RELEASE)
	{
	}
	else if (event_type == TIMER_ZERO)
	{
		LOG("Clock reached zero");
	}
	else if (event_type == STOPWATCH_ALARM)
	{
		Chrono* chrono = (Chrono*)element;
		LOG("Clock alarm at: %d", chrono->time);
	}

	return ret;
}