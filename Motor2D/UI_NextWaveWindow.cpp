#include "UI_NextWaveWindow.h"
#include "UI_Window.h"
#include "UI_Chrono.h"
#include "UI_Text.h"
#include "j1Fonts.h"
#include "j1WaveController.h"
#include "UI_Image.h"
#include "j1EntityController.h"
#include "UI_Button.h"

NextWaveWindow::NextWaveWindow(SDL_Texture* atlas, SDL_Texture* icon_atlas, Button* button, int x, int y, int min_x, int min_y, SDL_Rect section, int firstIcon_posX, int firstIcon_posY, int icons_offsetX, int icons_offsetY, j1Module* callback): UI_element(x, y, MENU, section, callback, atlas),
firstIcon_pos({firstIcon_posX, firstIcon_posY}),
icons_offset({icons_offsetX, icons_offsetY}),
icon_atlas(icon_atlas),
button(button),
default_pos({x,y}),
minimized_pos({min_x, min_y})
{
	window = new Window(texture, 0, 0, section, callback);
	window->parent = this;
	text = new Text("Next Wave in:", 10, 4, App->font->fonts.front(), { 255,255,255,255 }, nullptr);
	text->parent = this;
	timer = new Chrono(150, 4, TIMER, App->font->fonts.front(), { 255,255,255,255 }, nullptr);
	timer->parent = this;
	timer->setStartValue(App->wavecontroller->initial_wait);
	button->parent = this;
}

NextWaveWindow::~NextWaveWindow()
{
	RELEASE(window);
	RELEASE(text);
	RELEASE(timer);
	RELEASE(button);

	cleanIcons();
}

void NextWaveWindow::BlitElement()
{
	if (timer->time == 10 && minimized)
		toggle();

	window->BlitElement();
	text->BlitElement();
	timer->BlitElement();
	if (!minimized)
		button->flipVertical = true;
	else
		button->flipVertical = false;
	button->BlitElement();

	for (std::list<TroopIcon*>::iterator it_i = enemiesIcons.begin(); it_i != enemiesIcons.end(); it_i++)
	{
		(*it_i)->image->BlitElement();
	}

	for (std::list<Text*>::iterator it_t = squads.begin(); it_t != squads.end(); it_t++)
	{
		(*it_t)->BlitElement();
	}
}

void NextWaveWindow::updateWave()
{
	if (App->wavecontroller->current_wave > 0 && timer->start_value == App->wavecontroller->initial_wait)
		timer->setStartValue(App->wavecontroller->wait_between_waves);

	timer->restartChrono();

	cleanIcons();

	int counterX = 0;
	int counterY = 0;
	int gruntSquads = 0;
	int axeThrowerSquads = 0;
	for (std::list<NextWave*>::iterator it_e = App->wavecontroller->next_wave.begin(); it_e != App->wavecontroller->next_wave.end(); it_e++)
	{
		switch ((*it_e)->type)
		{
		case GRUNT:
			gruntSquads++;
			break;
		case AXE_THROWER:
			axeThrowerSquads++;
			break;
		}
	}

	
	if (gruntSquads > 0)
	{
		TroopIcon* enemyIcon = new TroopIcon(App->entitycontroller->getUnitFromDB(GRUNT), firstIcon_pos.x + (icons_offset.x*counterX), firstIcon_pos.y + (icons_offset.y*counterY));
		enemyIcon->image->parent = this;
		enemiesIcons.push_back(enemyIcon);

		Text* num = new Text(("x" + std::to_string(gruntSquads * App->entitycontroller->getUnitFromDB(GRUNT)->squad_members)), firstIcon_pos.x + 100, firstIcon_pos.y + (icons_offset.y*counterY) + 10, App->font->fonts.front(), { 0,0,0,255 }, nullptr);
		num->parent = this;
		squads.push_back(num);

		counterY++;
	}
	if (axeThrowerSquads > 0)
	{
		TroopIcon* enemyIcon = new TroopIcon(App->entitycontroller->getUnitFromDB(AXE_THROWER), firstIcon_pos.x + (icons_offset.x*counterX), firstIcon_pos.y + (icons_offset.y*counterY));
		enemyIcon->image->parent = this;
		enemiesIcons.push_back(enemyIcon);

		Text* num = new Text(("x" + std::to_string(axeThrowerSquads * App->entitycontroller->getUnitFromDB(AXE_THROWER)->squad_members)), firstIcon_pos.x + 100, firstIcon_pos.y + (icons_offset.y*counterY) + 15, App->font->fonts.front(), { 0,0,0,255 }, nullptr);
		num->parent = this;
		squads.push_back(num);

		counterY++;
	}
}

void NextWaveWindow::cleanIcons()
{
	std::list<TroopIcon*>::iterator it_i = enemiesIcons.begin();
	while (it_i != enemiesIcons.end())
	{
		RELEASE((*it_i));
		it_i++;
	}
	enemiesIcons.clear();

	std::list<Text*>::iterator it_t = squads.begin();
	while (it_t != squads.end())
	{
		RELEASE((*it_t));
		it_t++;
	}
	squads.clear();
}

void NextWaveWindow::toggle()
{
	minimized = !minimized;
	if (minimized)
		localPosition = minimized_pos;
	else
		localPosition = default_pos;
	button->state = STANDBY;
}

UI_element* NextWaveWindow::getMouseHoveringElement()
{
	UI_element* ret = this;

	if (App->gui->checkMouseHovering(button))
		ret = button;

	return ret;
}
