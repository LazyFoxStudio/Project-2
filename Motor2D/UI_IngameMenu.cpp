#include "UI_IngameMenu.h"
#include "j1App.h"
#include "j1Render.h"
#include "Brofiler\Brofiler.h"
#include "UI_Window.h"
#include "UI_Button.h"
#include "UI_Text.h"
#include "Entity.h"
#include "j1Fonts.h"
#include "j1EntityController.h"
#include "UI_SelectionDisplay.h"
#include "UI_InfoTable.h"

IngameMenu::IngameMenu(SDL_Texture* atlas, int x, int y, SDL_Rect section, int firstButton_posX, int firstButton_posY, int buttons_offsetX, int buttons_offsetY, j1Module* callback) : UI_element(x, y, element_type::MENU, section, callback, atlas),
firstButton_pos({firstButton_posX, firstButton_posY}),
buttons_offset({buttons_offsetX, buttons_offsetY})
{
	selectionDisplay = new SelectionDisplay();
	infoTable = new InfoTable();
	window = new Window(texture, x, y, section, callback);
	workers_title = new Text("Workers:", 1480, 815, App->font->fonts.front(), { 0,0,0,255 }, nullptr);
	workers = new Text("", 1495, 865, App->font->fonts.front(), { 0,0,0,255 }, nullptr);
}

IngameMenu::~IngameMenu()
{
	RELEASE(window);
	RELEASE(selectionDisplay);
	RELEASE(infoTable);
	RELEASE(workers_title)
	RELEASE(workers);
}

void IngameMenu::updateInfo()
{
	selectionDisplay->newSelection();
	infoTable->newSelection();
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
			if (counter >= 6) break;

			std::vector<Unit*> units;
			(*it_s)->getUnits(units);

			for (int i = 0; i < units.size(); i++)//for each entity of the squad
			{
				Unit* unit = units[i];

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
		if ((*it_b)->clickAction == ASSIGN_WORKER || (*it_b)->clickAction == UNASSIGN_WORKER) //HARDCODED
			extraYvalue = 50;
		else extraYvalue = 0;
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

void IngameMenu::BlitElement()
{
	BROFILER_CATEGORY("In-game Menu Blit", Profiler::Color::Beige);

	//Blit window
	window->BlitElement();

	//Blit selection display
	selectionDisplay->BlitElement();

	//Blit action butons
	for (std::list<Button*>::iterator it_b = actionButtons.begin(); it_b != actionButtons.end(); it_b++)
	{
		if ((*it_b)->active)
			(*it_b)->BlitElement();
	}

	//Blit info table
	infoTable->BlitElement();

	//HARDCODED ?
	//Blit workers on Lumber Mill
	if (workers_title != nullptr && workers != nullptr && App->entitycontroller->selected_entities.size() > 0 && App->entitycontroller->selected_entities.front()->type == LUMBER_MILL)
	{
		workers_title->BlitElement();
		int workers_num = ((Building*)App->entitycontroller->selected_entities.front())->workers_inside.size();
		std::string text = std::to_string(workers_num) + '/' + std::to_string(MAX_VILLAGERS_LUMBERMILL);
		workers->setText(text);
		workers->BlitElement();
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
