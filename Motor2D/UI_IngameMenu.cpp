#include "UI_IngameMenu.h"
#include "j1App.h"
#include "j1Render.h"
#include "Brofiler\Brofiler.h"
#include "UI_Window.h"
#include "UI_Image.h"
#include "UI_Button.h"
//#include "UI_Minimap.h"
#include "UI_Text.h"
#include "Entity.h"
#include "UI_LifeBar.h"
#include "j1Fonts.h"
#include "UI_ProgressBar.h"
#include "j1EntityController.h"

IngameMenu::IngameMenu(SDL_Texture* atlas, SDL_Texture* icon_atlas, int x, int y, SDL_Rect section, int minimap_posX, int minimap_posY, int firstIcon_posX, int firstIcon_posY, int icons_offsetX, int icons_offsetY, int lifeBars_offsetX, int lifeBars_offsetY, int stats_posX, int stats_posY, j1Module * callback) : UI_element(x, y, element_type::MENU, section, callback, atlas),
firstIcon_pos({firstIcon_posX, firstIcon_posY}),
icons_offset({icons_offsetX, icons_offsetY}),
lifeBars_offset({lifeBars_offsetX, lifeBars_offsetY}),
stats_pos({stats_posX, stats_posY}),
icon_atlas(icon_atlas)
{
	window = new Window(texture, x, y, section, callback);
	createStatsDisplay();
	//Create minimap
}

IngameMenu::~IngameMenu()
{
	cleanLists();
	//RELEASE(minimap);
	RELEASE(window);
}

void IngameMenu::updateInfo()
{
	cleanLists(true, true, false, true, false);
	createSelectionBasicInfo();
	updateStatsDisplay();
	updateActionButtons();
}

void IngameMenu::createSelectionBasicInfo()
{
	int counterX = 0;
	int counterY = 0;
	for (std::list<Entity*>::iterator it_e = App->entitycontroller->selected_entities.begin(); it_e != App->entitycontroller->selected_entities.end(); it_e++)
	{
		troopsIcons.push_back(new Image(icon_atlas, firstIcon_pos.x + icons_offset.x*counterX, firstIcon_pos.y + icons_offset.y*counterY, App->gui->GetIconRect((*it_e)), callback));
		(*troopsIcons.rbegin())->setBorder(true, White, 4);
		lifeBars.push_back(new LifeBar((*it_e), texture, firstIcon_pos.x + icons_offset.x*counterX + lifeBars_offset.x, firstIcon_pos.y + icons_offset.y*counterY + lifeBars_offset.y));
		counterY++;
		if (counterY == 3)
		{
			counterX++;
			counterY = 0;
		}
	}
}

void IngameMenu::createStatsDisplay()
{
	title = new Text("Squad Stats:", stats_pos.x, stats_pos.y, (*App->font->fonts.begin()), { 0,0,0,255 }, callback);
	title->active = false;

	statsTitles.push_back(new Text("Damage:", stats_pos.x + 20, stats_pos.y + 40, (*App->font->fonts.begin()), { 0,0,0,255 }, callback));
	(*statsTitles.rbegin())->active = false;
	statsTitles.push_back(new Text("Armor:", stats_pos.x + 20, stats_pos.y + 80, (*App->font->fonts.begin()), { 0,0,0,255 }, callback));
	(*statsTitles.rbegin())->active = false;
	statsTitles.push_back(new Text("Sight:", stats_pos.x + 20, stats_pos.y + 120, (*App->font->fonts.begin()), { 0,0,0,255 }, callback));
	(*statsTitles.rbegin())->active = false;
	statsTitles.push_back(new Text("Range:", stats_pos.x + 20, stats_pos.y + 160, (*App->font->fonts.begin()), { 0,0,0,255 }, callback));
	(*statsTitles.rbegin())->active = false;
	statsTitles.push_back(new Text("Speed:", stats_pos.x + 20, stats_pos.y + 200, (*App->font->fonts.begin()), { 0,0,0,255 }, callback));
	(*statsTitles.rbegin())->active = false;
}

void IngameMenu::updateStatsDisplay()
{
	
	if (App->entitycontroller->selected_entities.size() > 0)
	{
		Entity* entity = App->entitycontroller->selected_entities.front();
		if (entity->entity_type == entityType::UNIT)
		{
			for (std::list<Text*>::iterator it_t = statsTitles.begin(); it_t != statsTitles.end(); it_t++)
			{
				(*it_t)->active = true;
			}
			title->active = true;

			statsNumbers.push_back(new Text(std::to_string((int)((Unit*)entity)->attack + (int)((Unit*)entity)->piercing_atk), stats_pos.x + 170, stats_pos.y + 40, (*App->font->fonts.begin()), { 0,0,0,255 }, callback));
			statsNumbers.push_back(new Text(std::to_string((int)((Unit*)entity)->defense), stats_pos.x + 170, stats_pos.y + 80, (*App->font->fonts.begin()), { 0,0,0,255 }, callback));
			statsNumbers.push_back(new Text(std::to_string((int)((Unit*)entity)->line_of_sight), stats_pos.x + 170, stats_pos.y + 120, (*App->font->fonts.begin()), { 0,0,0,255 }, callback));
			statsNumbers.push_back(new Text(std::to_string((int)((Unit*)entity)->range), stats_pos.x + 170, stats_pos.y + 160, (*App->font->fonts.begin()), { 0,0,0,255 }, callback));
			statsNumbers.push_back(new Text(std::to_string((int)((Unit*)entity)->speed), stats_pos.x + 170, stats_pos.y + 200, (*App->font->fonts.begin()), { 0,0,0,255 }, callback));
		}
		else if (entity->entity_type == entityType::HERO)
		{
			for (std::list<Text*>::iterator it_t = statsTitles.begin(); it_t != statsTitles.end(); it_t++)
			{
				(*it_t)->active = true;
			}
			title->active = true;

			statsNumbers.push_back(new Text(std::to_string((int)((Hero*)entity)->attack + (int)((Hero*)entity)->piercing_atk), stats_pos.x + 170, stats_pos.y + 40, (*App->font->fonts.begin()), { 0,0,0,255 }, callback));
			statsNumbers.push_back(new Text(std::to_string((int)((Hero*)entity)->defense), stats_pos.x + 170, stats_pos.y + 80, (*App->font->fonts.begin()), { 0,0,0,255 }, callback));
			statsNumbers.push_back(new Text(std::to_string((int)((Hero*)entity)->line_of_sight), stats_pos.x + 170, stats_pos.y + 120, (*App->font->fonts.begin()), { 0,0,0,255 }, callback));
			statsNumbers.push_back(new Text(std::to_string((int)((Hero*)entity)->range), stats_pos.x + 170, stats_pos.y + 160, (*App->font->fonts.begin()), { 0,0,0,255 }, callback));
			statsNumbers.push_back(new Text(std::to_string((int)((Hero*)entity)->speed), stats_pos.x + 170, stats_pos.y + 200, (*App->font->fonts.begin()), { 0,0,0,255 }, callback));

		}
		else if(entity->entity_type == entityType::BUILDING)
		{
			for (std::list<Text*>::iterator it_t = statsTitles.begin(); it_t != statsTitles.end(); it_t++)
			{
				(*it_t)->active = false;
			}
			title->active = false;
		}
	}
	else
	{
		for (std::list<Text*>::iterator it_t = statsTitles.begin(); it_t != statsTitles.end(); it_t++)
		{
			(*it_t)->active = false;
		}
		title->active = false;
	}
}

void IngameMenu::updateActionButtons()
{

	if (App->entitycontroller->selected_entities.size() > 0)
	{
		Entity* entity = App->entitycontroller->selected_entities.front();
		actionButtons = App->gui->activateActionButtons(entity->available_actions);
	}
	else
	{
		uint a[9] = {};
		actionButtons = App->gui->activateActionButtons(a);
	}
}

void IngameMenu::cleanLists(bool icons, bool lifebars, bool statstitles, bool statsnumbers, bool buttons)
{
	if (icons)
	{
		//Clean troops icons
		std::list<Image*>::iterator it_i = troopsIcons.begin();
		while (it_i != troopsIcons.end())
		{
			RELEASE(*it_i);
			it_i++;
		}
		troopsIcons.clear();
	}
	if (lifebars)
	{
		//Clean life bars
		std::list<LifeBar*>::iterator it_l = lifeBars.begin();
		while (it_l != lifeBars.end())
		{
			RELEASE(*it_l);
			it_l++;
		}
		lifeBars.clear();
	}
	if (statstitles)
	{
		//Clean stats titles
		std::list<Text*>::iterator it_t = statsTitles.begin();
		while (it_t != statsTitles.end())
		{
			RELEASE(*it_t);
			it_t++;
		}
		statsTitles.clear();
	}
	if (statsnumbers)
	{
		//Clean stats numbers
		std::list<Text*>::iterator it_t = statsNumbers.begin();
		while (it_t != statsNumbers.end())
		{
			RELEASE(*it_t);
			it_t++;
		}
		statsNumbers.clear();
	}
}

void IngameMenu::BlitElement(bool use_camera)
{
	BROFILER_CATEGORY("In-game Menu Blit", Profiler::Color::Beige);

	//update minimap

	//Blit window
	window->BlitElement(use_camera);
	//Blit minimap
	//minimap->BlitElement(use_camera);
	//Blit icons
	for (std::list<Image*>::iterator it_i = troopsIcons.begin(); it_i != troopsIcons.end(); it_i++)
	{
		(*it_i)->BlitElement(use_camera);
	}
	//Blit life bars
	for (std::list<LifeBar*>::iterator it_l = lifeBars.begin(); it_l != lifeBars.end(); it_l++)
	{
		(*it_l)->BlitElement(use_camera);
	}
	//Blit action butons
	for (std::list<Button*>::iterator it_b = actionButtons.begin(); it_b != actionButtons.end(); it_b++)
	{
		if ((*it_b)->active)
			(*it_b)->BlitElement(use_camera);
	}
	//Blit stats
	if (title != nullptr && title->active)
		title->BlitElement(use_camera);
	for (std::list<Text*>::iterator it_t = statsTitles.begin(); it_t != statsTitles.end(); it_t++)
	{
		if ((*it_t)->active)
			(*it_t)->BlitElement(use_camera);
	}
	for (std::list<Text*>::iterator it_t = statsNumbers.begin(); it_t != statsNumbers.end(); it_t++)
	{
		if ((*it_t)->active)
			(*it_t)->BlitElement(use_camera);
	}
}

UI_element* IngameMenu::getMouseHoveringElement()
{
	UI_element* ret = (UI_element*)this;

	for (std::list<Button*>::iterator it_b = actionButtons.begin(); it_b != actionButtons.end(); it_b++)
	{
		if (App->gui->checkMouseHovering((*it_b)) && (*it_b)->interactive)
		{
			ret = (*it_b);
		}
	}

	return ret;
}
