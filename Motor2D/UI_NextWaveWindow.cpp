#include "UI_NextWaveWindow.h"
#include "UI_Window.h"
#include "UI_Chrono.h"
#include "UI_Text.h"
#include "j1Fonts.h"
#include "j1WaveController.h"
#include "UI_Image.h"

NextWaveWindow::NextWaveWindow(SDL_Texture* atlas, int x, int y, SDL_Rect section, int firstIcon_posX, int firstIcon_posY, int icons_offsetX, int icons_offsetY, j1Module* callback): UI_element(x, y, MENU, section, callback, atlas),
firstIcon_pos({firstIcon_posX, firstIcon_posY}),
icons_offset({icons_offsetX, icons_offsetY})
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

	std::list<TroopIcon*>::iterator it_i = enemiesIcons.begin();
	while (it_i != enemiesIcons.end())
	{
		RELEASE((*it_i));
		it_i++;
	}
	enemiesIcons.clear();
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
}

void NextWaveWindow::updateWave()
{
	if (timer->start_value == App->wavecontroller->initial_wait)
		timer->setStartValue(App->wavecontroller->wait_between_waves);

	timer->restartChrono();
}
