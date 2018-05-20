#include "UI_NextWaveWindow.h"
#include "UI_Window.h"
#include "UI_Chrono.h"
#include "UI_Text.h"
#include "j1Fonts.h"
#include "j1WaveController.h"
#include "UI_Image.h"
#include "j1EntityController.h"
#include "UI_Button.h"

NextWaveWindow::NextWaveWindow(SDL_Texture* atlas, SDL_Texture* icon_atlas, Button* button, int x, int y, int min_x, int min_y, SDL_Rect section, int firstIcon_posX, int firstIcon_posY, int icons_offsetX, int icons_offsetY, j1Module* callback): UI_element(min_x, min_y, MENU, section, callback, atlas),
firstIcon_pos({firstIcon_posX, firstIcon_posY}),
icons_offset({icons_offsetX, icons_offsetY}),
icon_atlas(icon_atlas),
button(button),
default_pos({x,y}),
minimized_pos({min_x, min_y})
{
	window = new Window(texture, 0, 0, section, callback);
	window->parent = this;
	text = new Text("Next Wave in:", 20, 8, App->font->fonts.front(), { 255,255,255,255 }, nullptr);
	//text->setOutlined(true);
	//text->setOutlineColor({ 0,0,0,255 });
	text->parent = this;
	timer = new Chrono(150, 8, TIMER, App->font->fonts.front(), { 255,255,255,255 }, nullptr);
	//timer->text->setOutlined(true);
	//timer->text->setOutlineColor({ 0,0,0,255 });
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
	counterX = counterY = 0;
	if (App->wavecontroller->current_wave > 0 && timer->start_value == App->wavecontroller->initial_wait)
		timer->setStartValue(App->wavecontroller->wait_between_waves);

	timer->restartChrono();

	cleanIcons();

	int gruntSquads = 0;
	int axeThrowerSquads = 0;
	int deathKnightSquads = 0;
	int dragonSquads = 0;
	int catapultSquads = 0;
	int juggernautSquads = 0;
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
		case DEATH_KNIGHT:
			deathKnightSquads++;
			break;
		case DRAGON:
			dragonSquads++;
			break;
		case CATAPULT:
			catapultSquads++;
			break;
		case JUGGERNAUT:
			juggernautSquads++;
			break;
		}
	}
	
	createIncomingEnemy(GRUNT, gruntSquads);
	createIncomingEnemy(AXE_THROWER, axeThrowerSquads);
	createIncomingEnemy(DEATH_KNIGHT, deathKnightSquads);
	createIncomingEnemy(DRAGON, dragonSquads);
	createIncomingEnemy(CATAPULT, catapultSquads);
	createIncomingEnemy(JUGGERNAUT, juggernautSquads);
}

bool NextWaveWindow::createIncomingEnemy(Type type, int amount)
{
	bool ret = false;
	
	if (amount > 0)
	{
		TroopIcon* enemyIcon = new TroopIcon(App->entitycontroller->getUnitFromDB(type), firstIcon_pos.x + (icons_offset.x*counterX), firstIcon_pos.y + (icons_offset.y*counterY));
		enemyIcon->image->parent = this;
		enemiesIcons.push_back(enemyIcon);

		Text* num = new Text(("x" + std::to_string(amount * App->entitycontroller->getUnitFromDB(type)->squad_members)), firstIcon_pos.x + (icons_offset.x*counterX) + 78, firstIcon_pos.y + (icons_offset.y*counterY) + 15, App->font->getFont(11), { 255,255,255,255 }, nullptr);
		num->parent = this;
		squads.push_back(num);
		
		counterY++;
		if (counterY == 3)
		{
			counterY = 0;
			counterX++;
		}

		ret = true;
	}

	return ret;
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
