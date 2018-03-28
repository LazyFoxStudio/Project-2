#include "UI_IngameMenu.h"
#include "j1App.h"
#include "j1Render.h"
#include "Brofiler\Brofiler.h"
#include "UI_Window.h"
#include "UI_Image.h"
#include "UI_Text.h"
#include "UI_Button.h"
#include "Entity.h"
#include "UI_LifeBar.h"
#include "j1EntityController.h"

IngameMenu::IngameMenu(SDL_Texture* atlas, SDL_Texture* icon_atlas, int x, int y, SDL_Rect section, int minimap_posX, int minimap_posY, int firstIcon_posX, int firstIcon_posY, int icons_offsetX, int icons_offsetY, int lifeBars_offsetX, int lifeBars_offsetY, int stats_posX, int stats_posY, int firstButton_posX, int firstButton_posY, int buttons_offsetX, int buttons_offsetY, j1Module * callback) : UI_element(x, y, element_type::MENU, section, callback, atlas),
firstIcon_pos({firstIcon_posX, firstIcon_posY}),
icons_offset({icons_offsetX, icons_offsetY}),
lifeBars_offset({lifeBars_offsetX, lifeBars_offsetY}),
stats_pos({stats_posX, stats_posY}),
firstButton_pos({firstButton_posX, firstButton_posY}),
buttons_offset({buttons_offsetX, buttons_offsetY}),
icon_atlas(icon_atlas)
{
	window = new Window(texture, x, y, section, callback);
	//Create minimap
}

void IngameMenu::newSelection()
{
}

void IngameMenu::cleanLists()
{
	std::list<Image*>::iterator it_i = troopsIcons.begin();
	while (it_i != troopsIcons.end())
	{
		RELEASE(*it_i);
		it_i++;
	}
	troopsIcons.clear();
}

void IngameMenu::BlitElement(bool use_camera)
{
	BROFILER_CATEGORY("In-game Menu Blit", Profiler::Color::Beige);

	//update minimap
	//update icons and life bars
	if (App->entitycontroller->newSelection)
	{
		int counterX = 0;
		int counterY = 0;
		cleanLists();
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

	//Blit window
	window->BlitElement(use_camera);
	//Blit minimap
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
}
