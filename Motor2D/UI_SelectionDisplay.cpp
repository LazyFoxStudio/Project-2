#include "UI_SelectionDisplay.h"
#include "j1App.h"
#include "j1Gui.h"
#include "UI_Image.h"
#include "UI_LifeBar.h"
#include "j1EntityController.h"
#include "Squad.h"

SelectionDisplay::~SelectionDisplay()
{
	cleanLists();
}

void SelectionDisplay::newSelection()
{
	cleanLists();

	if (App->entitycontroller->selected_squads.size() > 1)
		severalSquads = true;
	else
		severalSquads = false;

	iPoint position = FIRST_ICON_POSITION;
	iPoint icon_offset = TROOP_ICON_OFFSET;

	int counterX = 0;
	int counterY = 0;
	if (severalSquads)
	{
		for (std::list<Squad*>::iterator it_s = App->entitycontroller->selected_squads.begin(); it_s != App->entitycontroller->selected_squads.end(); it_s++)
		{
			squads.push_back(new SquadDisplay((*it_s), position.x + (icon_offset.x*counterX), position.y + (icon_offset.y*counterY)));
			counterY++;
			if (counterY == 3)
			{
				counterY = 0;
				counterX++;
			}
		}
	}
	else
	{
		for (std::list<Entity*>::iterator it_e = App->entitycontroller->selected_entities.begin(); it_e != App->entitycontroller->selected_entities.end(); it_e++)
		{
			troops.push_back(new TroopDisplay((*it_e), position.x + (icon_offset.x*counterX), position.y + (icon_offset.y*counterY)));
			counterY++;
			if (counterY == 3)
			{
				counterY = 0;
				counterX++;
			}
		}
	}
}

void SelectionDisplay::OrderDisplay()
{
	iPoint position = FIRST_ICON_POSITION;
	iPoint icon_offset = TROOP_ICON_OFFSET;
	iPoint lifeBar_offset = LIFEBARS_OFFSET;

	int counterX = 0;
	int counterY = 0;
	if (severalSquads)
	{
		for (std::list<SquadDisplay*>::iterator it_s = squads.begin(); it_s != squads.end(); it_s++)
		{
			for (int i = 0; i < (*it_s)->troopIcons.size(); i++)
			{
				(*it_s)->troopIcons[i]->image->localPosition = { position.x + (i*SQUAD_ICON_OFFSET) + (counterX*icon_offset.x), position.y + (counterY*icon_offset.y) };
			}
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
		for (std::list<TroopDisplay*>::iterator it_t = troops.begin(); it_t != troops.end(); it_t++)
		{
			(*it_t)->icon->image->localPosition = { position.x + (counterX*icon_offset.x), position.y + (counterY*icon_offset.y) };
			(*it_t)->lifeBar->localPosition = { position.x + (counterX*icon_offset.x) + lifeBar_offset.x, position.y + (counterY*icon_offset.y) + lifeBar_offset.y };
		}
	}
}

void SelectionDisplay::cleanLists()
{
	//Clean troops display
	std::list<TroopDisplay*>::iterator it_t = troops.begin();
	while (it_t != troops.end())
	{
		RELEASE(*it_t);
		it_t++;
	}
	troops.clear();

	//Clean squads icons
	std::list<SquadDisplay*>::iterator it_s = squads.begin();
	while (it_s != squads.end())
	{
		RELEASE(*it_s);
		it_s++;
	}
	squads.clear();
}

void SelectionDisplay::deleteDisplay(Entity* entity)
{
	for (std::list<TroopDisplay*>::iterator it_t = troops.begin(); it_t != troops.end(); it_t++)
	{
		if ((*it_t)->icon->entity == entity)
		{
			RELEASE((*it_t));
			troops.erase(it_t);
			break;
		}
	}

	int unitFound = false;
	for (std::list<SquadDisplay*>::iterator it_s = squads.begin(); it_s != squads.end(); it_s++)
	{
		for (std::vector<TroopIcon*>::iterator it_i = (*it_s)->troopIcons.begin(); it_i != (*it_s)->troopIcons.end(); it_i++)
		{
			if ((*it_i)->entity == entity)
			{
				RELEASE((*it_i));
				(*it_s)->troopIcons.erase(it_i);
				unitFound = true;
				break;
			}
		}
		if (unitFound)
		{
			if ((*it_s)->troopIcons.size() == 0)
				squads.erase(it_s);
			break;
		}
	}

	OrderDisplay();
}

void SelectionDisplay::BlitElement(bool use_camera)
{
	if (!severalSquads)
	{
		for (std::list<TroopDisplay*>::iterator it_t = troops.begin(); it_t != troops.end(); it_t++)
		{
			(*it_t)->Draw();
		}
	}
	else
	{
		for (std::list<SquadDisplay*>::iterator it_s = squads.begin(); it_s != squads.end(); it_s++)
		{
			(*it_s)->Draw();
		}
	}
}

TroopIcon::TroopIcon(Entity* entity, int x, int y)
{
	image = new Image(App->gui->icon_atlas, x, y, App->gui->GetIconRect(entity), nullptr);
	image->setBorder(true, White, 4);
	
	this->entity = entity;
}

void TroopIcon::updateColor()
{
	if (entity->current_HP < entity->max_HP*0.2)
		image->border_color = Red;
	else if (entity->current_HP < entity->max_HP*0.5)
		image->border_color = Yellow;
	else
		image->border_color = Green;
}

TroopDisplay::TroopDisplay(Entity* entity, int x, int y)
{
	iPoint lifeBar_offset = LIFEBARS_OFFSET;

	icon = new TroopIcon(entity, x, y);
	lifeBar = new LifeBar(entity, App->gui->atlas, x + lifeBar_offset.x, y + lifeBar_offset.y);
}

void TroopDisplay::Draw()
{
	icon->image->BlitElement();
	lifeBar->BlitElement();
}

SquadDisplay::SquadDisplay(Squad* squad, int x, int y)
{
	for (int i = 0; i < squad->units.size(); i++)
	{
		Unit* unit = squad->units[i];

		troopIcons.push_back(new TroopIcon(unit, x + (SQUAD_ICON_OFFSET*i), y));
		//TODO update iconBorder
	}
}

void SquadDisplay::Draw()
{
	for (int i = 0; i < troopIcons.size(); i++)
	{
		troopIcons[i]->updateColor();
		troopIcons[i]->image->BlitElement();
	}
}
