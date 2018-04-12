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

j1UIScene::j1UIScene() { name = "introscene";}

j1UIScene::~j1UIScene() {}


bool j1UIScene::Start()
{
	pugi::xml_document	Gui_config_file;
	pugi::xml_node		guiconfig;

	guiconfig = App->LoadFile(Gui_config_file, "Gui_config.xml");

	//App->audio->PlayMusic("Main_Theme.mp3");

	LoadUI(guiconfig);

	//Set resource counters
	Text* text_position_y = (Text*)App->gui->GetElement(TEXT, 0);
	text_position_y->convertIntoCounter(&App->scene->inactive_workers);

	Text* gold_display = (Text*)App->gui->GetElement(TEXT, 2);
	gold_display->convertIntoCounter(&App->scene->workers);

	Text* wood_display = (Text*)App->gui->GetElement(TEXT, 3);
	wood_display->convertIntoCounter(&App->scene->wood);

	Text* wood_sec = (Text*)App->gui->GetElement(TEXT, 4);
	wood_sec->convertIntoCounter(&App->scene->wood_production_per_second);

	Text* waves = (Text*)App->gui->GetElement(TEXT, 6);
	waves->convertIntoCounter(&App->wavecontroller->current_wave);

	//Hardcoded
	Button* barracks = App->gui->GetActionButton(5);
	barracks->setCondition("Build first a Lumber Mill");
	barracks->Lock();
	Button* farms = App->gui->GetActionButton(7);
	farms->setCondition("Build first a Lumber Mill");
	farms->Lock();

	return true;
}


bool j1UIScene::Update(float dt)
{
	iPoint mouse_test;

	App->input->GetMousePosition(mouse_test.x, mouse_test.y);
	mouse_test=App->render->ScreenToWorld(mouse_test.x, mouse_test.y);
	
	x = mouse_test.x;
	y = mouse_test.y;

	//minimap_
	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT)
	{
		int camx, camy;
		minimap->Mouse_to_map(camx, camy);

		if (camx != -1 && camy != -1)
		{
		
			App->render->camera.y = -camy + App->render->camera.h / 2;
			App->render->camera.x = -camx + App->render->camera.w / 2;

			if (camx- App->render->camera.w/2 > 4096 - App->render->camera.w) {
				LOG("TOP RIGHT");
				App->render->camera.x = -4096 + App->render->camera.w;
			}
			else if (camx - App->render->camera.w / 2 < 0) {
				LOG("TOP LEFT");
				App->render->camera.x = 0;
			}

			if (camy - App->render->camera.h/2 > 4096 - App->render->camera.h){
				LOG("TOP DOWN");
				App->render->camera.y = -4096 + App->render->camera.h;
			}
			else if (camy - App->render->camera.h / 2 < 0) {
				LOG("TOP UP");
				App->render->camera.y = 0;
			}

		}
	}

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
	minimap->~Minimap();
	return true;
}

bool j1UIScene::OnUIEvent(UI_element* element, event_type event_type)
{
	bool ret = true;

	if (event_type == MOUSE_ENTER)
	{
		App->gui->hovering_element.Start();
		App->gui->current_hovering_element = element;
		if (element->state != LOCKED)
			element->state = MOUSEOVER;
		else
			element->state = LOCKED_MOUSEOVER;

	}
	else if (event_type == MOUSE_LEAVE)
	{
		if (element->state != LOCKED_MOUSEOVER)
			element->state = STANDBY;
		else
			element->state = LOCKED;

		element->blitPopUpInfo = false;
		App->gui->current_hovering_element = nullptr;
	}
	else if (event_type == MOUSE_LEFT_CLICK && element->state != LOCKED && element->state != LOCKED_MOUSEOVER)
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
	else if (event_type == MOUSE_LEFT_RELEASE && element->state != LOCKED && element->state != LOCKED_MOUSEOVER)
	{
		if (element->state == CLICKED)
			element->state = MOUSEOVER;

		switch (element->function)
		{
		case MOVE_FUNCTION:
			App->actionscontroller->activateAction(MOVE);
			break;
		case BUILD_BARRACKS_FUNCTION:
			App->actionscontroller->activateAction(BUILD_BARRACKS);
			break;
		case BUILD_LUMBER_MILL_FUNCTION:
			App->actionscontroller->activateAction(BUILD_LUMBER_MILL);
			break;
		case BUILD_FARM_FUNCTION:
			App->actionscontroller->activateAction(BUILD_FARM);
			break;
		case UNASSIGN_WORKER_FUNCTION:
			App->actionscontroller->activateAction(UNASSIGN_WORKER);
			break;
		case ASSIGN_WORKER_FUNCTION:
			App->actionscontroller->activateAction(ASSIGN_WORKER);
			break;
		case CREATE_FOOTMAN_FUNCTION:
			App->actionscontroller->activateAction(CREATE_FOOTMAN);
			break;
		case CREATE_ARCHER_FUNCTION:
			App->actionscontroller->activateAction(CREATE_ARCHER);
			break;
		}
	}
	else if (event_type == MOUSE_RIGHT_CLICK && element->state != LOCKED && element->state != LOCKED_MOUSEOVER)
	{
	}
	else if (event_type == MOUSE_RIGHT_RELEASE && element->state != LOCKED && element->state != LOCKED_MOUSEOVER)
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