#include "UI_NextWaveWindow.h"
#include "UI_Window.h"
#include "UI_Chrono.h"
#include "UI_Text.h"
#include "j1Fonts.h"
#include "j1WaveController.h"
#include "UI_Image.h"

NextWaveWindow::NextWaveWindow(SDL_Texture* atlas, SDL_Texture* icon_atlas, int x, int y, SDL_Rect section, int firstIcon_posX, int firstIcon_posY, int icons_offsetX, int icons_offsetY, j1Module* callback): UI_element(x, y, MENU, section, callback, atlas),
firstIcon_pos({firstIcon_posX, firstIcon_posY}),
icons_offset({icons_offsetX, icons_offsetY}),
icon_atlas(icon_atlas)
{
	window = new Window(texture, x, y, section, callback);
	text = new Text("Next Wave in:", x+10, y+4, App->font->fonts.front(), { 255,255,255,255 }, nullptr);
	timer = new Chrono(x+200, y+4, TIMER, App->font->fonts.front(), { 255,255,255,255 }, nullptr);
	timer->setStartValue(App->wavecontroller->initial_wait);
}

NextWaveWindow::~NextWaveWindow()
{
	RELEASE(window);
	RELEASE(text);
	RELEASE(timer);

	cleanIcons();
}

void NextWaveWindow::BlitElement(bool use_camera)
{
	window->BlitElement(use_camera);
	text->BlitElement(use_camera);
	timer->BlitElement(use_camera);

	for (std::list<TroopIcon*>::iterator it_i = enemiesIcons.begin(); it_i != enemiesIcons.end(); it_i++)
	{
		(*it_i)->image->BlitElement(use_camera);
	}

	for (std::list<Text*>::iterator it_t = squads.begin(); it_t != squads.end(); it_t++)
	{
		(*it_t)->BlitElement(use_camera);
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
		TroopIcon* enemyIcon = new TroopIcon();
		Image* img = new Image(icon_atlas, firstIcon_pos.x + (icons_offset.x*counterX), firstIcon_pos.y + (icons_offset.y*counterY), App->gui->GetUnitRect(GRUNT), callback);
		img->setBorder(true);
		enemyIcon->image = img;
		enemiesIcons.push_back(enemyIcon);

		Text* num = new Text(("x" + std::to_string(gruntSquads)), firstIcon_pos.x + 100, firstIcon_pos.y + (icons_offset.y*counterY) + 10, App->font->fonts.front(), { 0,0,0,255 }, nullptr);
		squads.push_back(num);

		counterY++;
	}
	if (axeThrowerSquads > 0)
	{
		TroopIcon* enemyIcon = new TroopIcon();
		Image* img = new Image(icon_atlas, firstIcon_pos.x + (icons_offset.x*counterX), firstIcon_pos.y + (icons_offset.y*counterY), App->gui->GetUnitRect(AXE_THROWER), callback);
		img->setBorder(true);
		enemyIcon->image = img;
		enemiesIcons.push_back(enemyIcon);

		Text* num = new Text(("x" + std::to_string(axeThrowerSquads)), firstIcon_pos.x + 100, firstIcon_pos.y + (icons_offset.y*counterY) + 15, App->font->fonts.front(), { 0,0,0,255 }, nullptr);
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
