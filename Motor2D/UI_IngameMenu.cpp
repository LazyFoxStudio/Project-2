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

void IngameMenu::BlitElement(bool use_camera)
{
	BROFILER_CATEGORY("In-game Menu Blit", Profiler::Color::Beige);

	//Blit window
	window->BlitElement(use_camera);

	//Blit selection display
	selectionDisplay->BlitElement(use_camera);

	//Blit action butons
	for (std::list<Button*>::iterator it_b = actionButtons.begin(); it_b != actionButtons.end(); it_b++)
	{
		if ((*it_b)->active)
			(*it_b)->BlitElement(use_camera);
	}

	//Blit info table
	infoTable->BlitElement();

	//HARDCODED ?
	//Blit workers on Lumber Mill
	if (workers_title != nullptr && workers != nullptr && App->entitycontroller->selected_entities.size() > 0 && App->entitycontroller->selected_entities.front()->type == LUMBER_MILL)
	{
		workers_title->BlitElement(use_camera);
		int workers_num = ((Building*)App->entitycontroller->selected_entities.front())->workers_inside.size();
		std::string text = std::to_string(workers_num) + '/' + std::to_string(MAX_VILLAGERS_LUMBERMILL);
		workers->setText(text);
		workers->BlitElement(use_camera);
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
