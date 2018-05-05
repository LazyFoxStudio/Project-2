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
#include "UI_Button.h"
#include "UI_Text.h"
#include "j1Fonts.h"
#include "j1Input.h"
#include "j1ActionsController.h"
#include "UI_CostDisplay.h"
#include "j1WaveController.h"

j1UIScene::j1UIScene() { name = "introscene"; pausable = false; }

j1UIScene::~j1UIScene() {}

bool j1UIScene::Start()
{
	pugi::xml_document	Gui_config_file;
	pugi::xml_node		guiconfig;

	guiconfig = App->LoadFile(Gui_config_file, "Gui_config.xml");

	//App->audio->PlayMusic("Main_Theme.mp3");

	LoadUI(guiconfig);

	//Set resource counters
	Text* text_position_y = (Text*)App->gui->GetElement(TEXT, 1);
	text_position_y->convertIntoCounter(&App->scene->inactive_workers_int);

	Text* gold_display = (Text*)App->gui->GetElement(TEXT, 3);
	gold_display->convertIntoCounter(&App->scene->workers_int);

	Text* wood_display = (Text*)App->gui->GetElement(TEXT, 4);
	wood_display->convertIntoCounter(&App->scene->wood);

	Text* wood_sec = (Text*)App->gui->GetElement(TEXT, 5);
	wood_sec->convertIntoCounter(&App->scene->wood_production_per_second);

	Text* waves = (Text*)App->gui->GetElement(TEXT, 7);
	waves->convertIntoCounter(&App->wavecontroller->current_wave);

	Text* survived_waves = (Text*)App->gui->GetElement(TEXT, 8);
	survived_waves->convertIntoCounter(&App->wavecontroller->current_wave);

	//Hardcoded
	Button* barracks = App->gui->GetActionButton(5);
	barracks->setCondition("Build first a Lumber Mill");
	barracks->Lock();
	Button* farms = App->gui->GetActionButton(7);
	farms->setCondition("Build first a Lumber Mill");
	farms->Lock();
	Button* mine = App->gui->GetActionButton(22);
	mine->setCondition("Build first a Lumber Mill");
	mine->Lock();
	Button* turret = App->gui->GetActionButton(23);
	turret->setCondition("Build first a Lumber Mill");
	turret->Lock();
	Button* hut = App->gui->GetActionButton(24);
	hut->setCondition("Build first a Lumber Mill");
	hut->Lock();
	Button* church = App->gui->GetActionButton(25);
	church->setCondition("Build first a Lumber Mill");
	church->Lock();
	Button* blacksmith = App->gui->GetActionButton(26);
	blacksmith->setCondition("Build first a Lumber Mill");
	blacksmith->Lock();

	return true;
}


bool j1UIScene::Update(float dt)
{

	if (App->input->GetKey(SDL_SCANCODE_U) == KEY_DOWN)
		App->pauseGame();
	if (App->input->GetKey(SDL_SCANCODE_J) == KEY_DOWN)
		App->resumeGame();

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
		menu* newMenu = new menu((menu_id)menuconfig.attribute("type").as_int(0));

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
		App->gui->hovering_element.Start();
		App->gui->current_hovering_element = element;
		if (element->state != LOCKED && element->state != LOCKED_MOUSEOVER)
			element->state = MOUSEOVER;
		else
			element->state = LOCKED_MOUSEOVER;
	}
	else if (event_type == MOUSE_LEAVE)
	{
		if (element->state != LOCKED_MOUSEOVER && element->state != LOCKED)
			element->state = STANDBY;
		else
			element->state = LOCKED;

		element->blitPopUpInfo = false;
		App->gui->current_hovering_element = nullptr;
	}
	else if (element->state != LOCKED && element->state != LOCKED_MOUSEOVER)
	{
		if (event_type == MOUSE_LEFT_CLICK)
		{
			element->state = CLICKED;

			if (element->hasClickAction)
				App->actionscontroller->activateAction(element->clickAction);
		}
		else if (event_type == MOUSE_LEFT_RELEASE)
		{
			if (element->state == CLICKED)
				element->state = MOUSEOVER;

			if (element->hasReleaseAction)
				App->actionscontroller->activateAction(element->releaseAction);
		}
		else if (event_type == MOUSE_RIGHT_CLICK)
		{}
		else if (event_type == MOUSE_RIGHT_RELEASE)
		{}
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

menu* j1UIScene::getMenu(menu_id id)
{
	menu* ret = nullptr;

	for (std::list<menu*>::iterator it_m = menus.begin(); it_m != menus.end(); it_m++)
	{
		if ((*it_m)->id == id)
		{
			ret = (*it_m);
			break;
		}
	}

	return ret;
}

void j1UIScene::toggleMenu(bool setActive, menu_id id)
{
	for (std::list<menu*>::iterator it_m = menus.begin(); it_m != menus.end(); it_m++)
	{
		if ((*it_m)->id == id)
		{
			(*it_m)->active = setActive;
			break;
		}
	}
}
