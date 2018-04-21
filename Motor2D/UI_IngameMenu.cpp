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
#include "Building.h"
#include "UI_LifeBar.h"
#include "j1Fonts.h"
#include "UI_ProgressBar.h"
#include "j1EntityController.h"
#include "Squad.h"

IngameMenu::IngameMenu(SDL_Texture* atlas, SDL_Texture* icon_atlas, int x, int y, SDL_Rect section, int minimap_posX, int minimap_posY, int firstIcon_posX, int firstIcon_posY, int icons_offsetX, int icons_offsetY, int squadIcon_offsetX, int lifeBars_offsetX, int lifeBars_offsetY, int stats_posX, int stats_posY, int firstButton_posX, int firstButton_posY, int buttons_offsetX, int buttons_offsetY, j1Module * callback) : UI_element(x, y, element_type::MENU, section, callback, atlas),
firstIcon_pos({firstIcon_posX, firstIcon_posY}),
icons_offset({icons_offsetX, icons_offsetY}),
squadIcon_offsetX(squadIcon_offsetX),
lifeBars_offset({lifeBars_offsetX, lifeBars_offsetY}),
stats_pos({stats_posX, stats_posY}),
firstButton_pos({firstButton_posX, firstButton_posY}),
buttons_offset({buttons_offsetX, buttons_offsetY}),
icon_atlas(icon_atlas)
{
	window = new Window(texture, x, y, section, callback);
	createStatsDisplay();
	workers = new Text("", 1495, 865, App->font->fonts.front(), { 0,0,0,255 }, nullptr);
	workers->active = false;
	//Create minimap
}

IngameMenu::~IngameMenu()
{
	cleanLists();
	//RELEASE(minimap);
	RELEASE(window);
	RELEASE(workers);
}

void IngameMenu::updateInfo()
{
	if (App->entitycontroller->selected_squads.size() > 1)
		severalSquads = true;
	else
		severalSquads = false;

	cleanLists(true, true, true, false, true, false);
	createSelectionBasicInfo();
	updateStatsDisplay();
	updateActionButtons();
}

void IngameMenu::createSelectionBasicInfo()
{
	int counterX = 0;
	int counterY = 0;
	if (!severalSquads)
	{
		for (std::list<Entity*>::iterator it_e = App->entitycontroller->selected_entities.begin(); it_e != App->entitycontroller->selected_entities.end(); it_e++)
		{
			TroopIcon* icon = new TroopIcon();

			Image* img = new Image(icon_atlas, firstIcon_pos.x + icons_offset.x*counterX, firstIcon_pos.y + icons_offset.y*counterY, App->gui->GetIconRect((*it_e)), callback);
			img->setBorder(true, White, 4);
			
			icon->image = img;
			icon->entity = (*it_e);
			troopsIcons.push_back(icon);
			
			lifeBars.push_back(new LifeBar((*it_e), texture, firstIcon_pos.x + icons_offset.x*counterX + lifeBars_offset.x, firstIcon_pos.y + icons_offset.y*counterY + lifeBars_offset.y));
			counterY++;
			if (counterY == 3)
			{
				counterX++;
				counterY = 0;
			}
		}
	}
	else
	{
		iPoint position = firstIcon_pos;
		int counter = 0;
		for (std::list<Squad*>::iterator it_s = App->entitycontroller->selected_squads.begin(); it_s != App->entitycontroller->selected_squads.end(); it_s++)
		{
			if (counter >= 6)
				break;
			for (int i = 0; i < (*it_s)->units.size(); i++)//for each entity of the squad
			{
				Unit* unit = (*it_s)->units[i];

				TroopIcon* icon = new TroopIcon();

				Image* img = new Image(icon_atlas, position.x + squadIcon_offsetX*counterX, position.y + icons_offset.y*counterY, App->gui->GetIconRect(unit), callback);
				Color borderColor = Green;
				int current_HP = unit->current_HP;
				int HP_percentage = unit->max_HP / 100;
				if (current_HP < (HP_percentage*0.5))
					borderColor = Yellow;
				else if (current_HP < (HP_percentage*0.2))
					borderColor = Red;
				img->setBorder(true, borderColor, 4);

				icon->image = img;
				icon->entity = unit;

				squadTroopsIcons.push_back(icon);

				counterX++;
			}
			counterY++;
			counterX = 0;
			if (counterY == 3)
			{
				position.x += icons_offset.x;
				counterY = 0;
			}
			counter++;
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
		if (entity->IsUnit() && !entity->IsUnit())
		{
			for (std::list<Text*>::iterator it_t = statsTitles.begin(); it_t != statsTitles.end(); it_t++)
			{
				(*it_t)->active = true;
			}
			title->setText("Squad Stats:");
			title->localPosition = { stats_pos.x, stats_pos.y };
			title->active = true;
			workers->active = false;

			statsNumbers.push_back(new Text(std::to_string((int)((Unit*)entity)->attack + (int)((Unit*)entity)->piercing_atk), stats_pos.x + 170, stats_pos.y + 40, (*App->font->fonts.begin()), { 0,0,0,255 }, callback));
			statsNumbers.push_back(new Text(std::to_string((int)((Unit*)entity)->defense), stats_pos.x + 170, stats_pos.y + 80, (*App->font->fonts.begin()), { 0,0,0,255 }, callback));
			statsNumbers.push_back(new Text(std::to_string((int)((Unit*)entity)->line_of_sight), stats_pos.x + 170, stats_pos.y + 120, (*App->font->fonts.begin()), { 0,0,0,255 }, callback));
			statsNumbers.push_back(new Text(std::to_string((int)((Unit*)entity)->range), stats_pos.x + 170, stats_pos.y + 160, (*App->font->fonts.begin()), { 0,0,0,255 }, callback));
			statsNumbers.push_back(new Text(std::to_string((int)((Unit*)entity)->speed), stats_pos.x + 170, stats_pos.y + 200, (*App->font->fonts.begin()), { 0,0,0,255 }, callback));
		}
		else if (entity->IsHero())
		{
			for (std::list<Text*>::iterator it_t = statsTitles.begin(); it_t != statsTitles.end(); it_t++)
			{
				(*it_t)->active = true;
			}
			title->setText("Hero Stats:");
			title->localPosition = { stats_pos.x, stats_pos.y };
			title->active = true;
			workers->active = false;

			statsNumbers.push_back(new Text(std::to_string((int)((Hero*)entity)->attack + (int)((Hero*)entity)->piercing_atk), stats_pos.x + 170, stats_pos.y + 40, (*App->font->fonts.begin()), { 0,0,0,255 }, callback));
			statsNumbers.push_back(new Text(std::to_string((int)((Hero*)entity)->defense), stats_pos.x + 170, stats_pos.y + 80, (*App->font->fonts.begin()), { 0,0,0,255 }, callback));
			statsNumbers.push_back(new Text(std::to_string((int)((Hero*)entity)->line_of_sight), stats_pos.x + 170, stats_pos.y + 120, (*App->font->fonts.begin()), { 0,0,0,255 }, callback));
			statsNumbers.push_back(new Text(std::to_string((int)((Hero*)entity)->range), stats_pos.x + 170, stats_pos.y + 160, (*App->font->fonts.begin()), { 0,0,0,255 }, callback));
			statsNumbers.push_back(new Text(std::to_string((int)((Hero*)entity)->speed), stats_pos.x + 170, stats_pos.y + 200, (*App->font->fonts.begin()), { 0,0,0,255 }, callback));

		}
		else if(entity->IsBuilding())
		{
			for (std::list<Text*>::iterator it_t = statsTitles.begin(); it_t != statsTitles.end(); it_t++)
			{
				(*it_t)->active = false;
			}
			if (((Building*)entity)->type == LUMBER_MILL)
			{
				title->setText("Workers:");
				title->localPosition = { stats_pos.x + 365, stats_pos.y + 20 };
				workers->active = true;
				title->active = true;
			}
			else
			{
				title->active = false;
				workers->active = false;
			}
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
	actionButtons.clear();

	if (App->entitycontroller->selected_entities.size() > 0)
	{
		Entity* entity = App->entitycontroller->selected_entities.front();
		actionButtons = App->gui->activateActionButtons(entity->available_actions);
	}
	else
	{
		std::vector<uint> empty_vector;
		actionButtons = App->gui->activateActionButtons(empty_vector);
	}

	int counterX = 0;
	int counterY = 0;
	int extraYvalue = 0;
	for (std::list<Button*>::iterator it_b = actionButtons.begin(); it_b != actionButtons.end(); it_b++)
	{
		if ((*it_b)->function == ASSIGN_WORKER_FUNCTION || (*it_b)->function == UNASSIGN_WORKER_FUNCTION)
			extraYvalue = 50;
		(*it_b)->localPosition.x = firstButton_pos.x + (buttons_offset.x*counterX);
		(*it_b)->localPosition.y = firstButton_pos.y + (buttons_offset.y*counterY) + extraYvalue;
		(*it_b)->active = true;
		(*it_b)->updatedPosition();
		counterX++;
		if (counterX == 3)
		{
			counterY++;
			counterX = 0;
		}
	}
}

void IngameMenu::updateSquadIcons()
{
	for (std::list<TroopIcon*>::iterator it_i = squadTroopsIcons.begin(); it_i != squadTroopsIcons.end(); it_i++)
	{
		Unit* unit = ((Unit*)(*it_i)->entity);
		if (unit->current_HP < unit->max_HP*0.2)
			(*it_i)->image->border_color = Red;
		else if (unit->current_HP < unit->max_HP*0.5)
			(*it_i)->image->border_color = Yellow;
		else
			(*it_i)->image->border_color = Green;
	}
}

void IngameMenu::cleanLists(bool icons, bool squadIcons, bool lifebars, bool statstitles, bool statsnumbers, bool buttons)
{
	if (icons)
	{
		//Clean troops icons
		std::list<TroopIcon*>::iterator it_i = troopsIcons.begin();
		while (it_i != troopsIcons.end())
		{
			RELEASE(*it_i);
			it_i++;
		}
		troopsIcons.clear();
	}
	if (squadIcons)
	{
		//Clean squad troops icons
		std::list<TroopIcon*>::iterator it_i = squadTroopsIcons.begin();
		while (it_i != squadTroopsIcons.end())
		{
			RELEASE(*it_i);
			it_i++;
		}
		squadTroopsIcons.clear();
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

void IngameMenu::deleteMenuTroop(Entity* entity)
{
	//Delete the icon
	std::list<TroopIcon*>::iterator it_i;
	if (severalSquads)
		deleteTroopIcon(entity, squadTroopsIcons);
	else
		deleteTroopIcon(entity, troopsIcons);

	//Delete the life bar
	App->gui->deleteLifeBar(entity, lifeBars);

	OrderSelectionIcons();
}

void IngameMenu::deleteTroopIcon(Entity* entity, std::list<TroopIcon*>& list)
{
	std::list<TroopIcon*>::iterator it_i = list.begin();
	while (it_i != list.end())
	{
		if ((*it_i)->entity == entity)
		{
			list.erase(it_i);
			RELEASE((*it_i));
			break;
		}
		it_i++;
	}
}

void IngameMenu::OrderSelectionIcons()
{
	int counterX = 0;
	int counterY = 0;
	if (!severalSquads)
	{
		for (std::list<TroopIcon*>::iterator it_i = troopsIcons.begin(); it_i != troopsIcons.end(); it_i++)
		{
			(*it_i)->image->localPosition = { firstIcon_pos.x + icons_offset.x*counterX, firstIcon_pos.y + icons_offset.y*counterY };
			counterY++;
			if (counterY == 3)
			{
				counterX++;
				counterY = 0;
			}
		}
		counterX = 0;
		counterY = 0;
		for (std::list<LifeBar*>::iterator it_l = lifeBars.begin(); it_l != lifeBars.end(); it_l++)
		{
			(*it_l)->bar->localPosition = { firstIcon_pos.x + icons_offset.x*counterX + lifeBars_offset.x, firstIcon_pos.y + icons_offset.y*counterY + lifeBars_offset.y };
			counterY++;
			if (counterY == 3)
			{
				counterX++;
				counterY = 0;
			}
		}		
	}
	else
	{
		if (squadTroopsIcons.size() > 0)
		{
			iPoint position = firstIcon_pos;
			Squad* common_squad = ((Unit*)(*squadTroopsIcons.begin())->entity)->squad;
			for (std::list<TroopIcon*>::iterator it_i = squadTroopsIcons.begin(); it_i != squadTroopsIcons.end(); it_i++)
			{
				if (common_squad != ((Unit*)(*it_i)->entity)->squad)
				{
					common_squad = ((Unit*)(*it_i)->entity)->squad;
					counterY++;
					counterX = 0;
					if (counterY == 3)
					{
						position.x += icons_offset.x;
						counterY = 0;
					}
				}
				(*it_i)->image->localPosition = { position.x + squadIcon_offsetX*counterX, position.y + icons_offset.y*counterY };
				counterX++;				
			}
		}
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
	if (severalSquads)
	{
		updateSquadIcons();
		for (std::list<TroopIcon*>::iterator it_i = squadTroopsIcons.begin(); it_i != squadTroopsIcons.end(); it_i++)
		{
			(*it_i)->image->BlitElement(use_camera);
		}
	}
	else
	{
		for (std::list<TroopIcon*>::iterator it_i = troopsIcons.begin(); it_i != troopsIcons.end(); it_i++)
		{
			(*it_i)->image->BlitElement(use_camera);
		}
		//Blit life bars
		for (std::list<LifeBar*>::iterator it_l = lifeBars.begin(); it_l != lifeBars.end(); it_l++)
		{
			(*it_l)->BlitElement(use_camera);
		}
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
	if (workers != nullptr && workers->active)
	{
		if (App->entitycontroller->selected_entities.size() > 0)
		{
			int workers_num = ((Building*)App->entitycontroller->selected_entities.front())->workers_inside;
			std::string text = std::to_string(workers_num) + '/' + std::to_string(MAX_VILLAGERS_LUMBERMILL);
			workers->setText(text);
			workers->BlitElement(use_camera);
		}
	}
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
