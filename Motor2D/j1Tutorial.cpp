#include "j1Tutorial.h"
#include "UI_TextBox.h"
#include "UI_Image.h"
#include "UI_element.h"
#include "j1App.h"
#include "j1Gui.h"
#include "j1EntityController.h"
#include "UI_NextWaveWindow.h"
#include "UI_Chrono.h"
#include "j1WaveController.h"

j1Tutorial::j1Tutorial()
{
}


j1Tutorial::~j1Tutorial()
{
}

bool j1Tutorial::Start()
{
	loadTutorial("tutorial.xml");

	arrow = new Image(App->gui->atlas, 0, 0, { 991, 809, 149, 113 }, nullptr);
	arrow->use_camera = true;

	return true;
}

bool j1Tutorial::Update(float dt)
{
	if (doingTutorial)
	{
		if (missing_steps.size() == 0 && activeStep == nullptr)
		{
			doingTutorial = false;
			active = false;
			App->wavecontroller->wave_timer.Start();
			App->gui->nextWaveWindow->timer->counter.Start();
			return true;
		}
		if (activeStep == nullptr)
		{
			activeStep = missing_steps.front();
			if (activeStep->isUI)
				arrow->use_camera = false;
			else
				arrow->use_camera = true;
			missing_steps.remove(activeStep);
			timer.Start();
		}
	}

	return true;
}

bool j1Tutorial::PostUpdate()
{
	if (doingTutorial && activeStep != nullptr)
	{
		activeStep->Draw();
		if (activeStep->isFinished())
			activeStep = nullptr;
	}

	return true;
}

void j1Tutorial::loadTutorial(char* path)
{
	pugi::xml_document tutorialDoc;
	pugi::xml_node tutorial = App->LoadFile(tutorialDoc, path);

	if (tutorial)
	{
		for (pugi::xml_node stepN = tutorial.child("step"); stepN; stepN = stepN.next_sibling("step"))
		{
			Step* step = new Step((Task)stepN.attribute("task").as_int() , stepN.attribute("duration").as_int());
			step->isUI = stepN.attribute("UI").as_bool();
			pugi::xml_node textN = stepN.child("text");
			if (textN)
			{
				TextBox* text = new TextBox(textN.attribute("x").as_int(), textN.attribute("y").as_int(), 0, 0);
				if (!step->isUI)
					text->use_camera = true;
				for (pugi::xml_node line = textN.child("line"); line; line = line.next_sibling("line"))
				{
					text->addTextLine(line.attribute("text").as_string());
				}				
				step->text = text;
			}
			pugi::xml_node arrowN = stepN.child("arrow");
			if (arrowN)
			{
				ArrowInfo* info = new ArrowInfo();
				info->pointAt = { arrowN.attribute("x").as_int(), arrowN.attribute("y").as_int() };
				info->rotation = arrowN.attribute("rotation").as_int();
				step->arrowInfo = info;
			}
			steps.push_back(step);
		}
	}
}

void j1Tutorial::startTutorial()
{
	if (!doingTutorial)
	{
		missing_steps = steps;
		doingTutorial = true;
		App->wavecontroller->tutorial = true;
		timer.Start();
	}
}

void j1Tutorial::finishStep()
{
	if (activeStep != nullptr)
		activeStep = nullptr;
}

void j1Tutorial::taskCompleted(Task task)
{
	if (activeStep->task == task)
		finishStep();

	if (task == PLACE_FARM && !builded)
	{
		App->entitycontroller->addHero(iPoint(2000, 1950), HERO_1);
		App->wavecontroller->forceNextWave();
		App->gui->nextWaveWindow->timer->counter.PauseTimer();
		builded = true;
	}
	else if (task == SELECT_HERO)
	{
		App->wavecontroller->wave_timer.PauseTimer();
	}
	else if (task == KILL_ENEMIES)
	{
		App->gui->nextWaveWindow->active = true;
		App->gui->nextWaveWindow->toggle();
	}
}

void Step::Draw()
{
	if (text != nullptr)
		text->BlitElement();
	if (arrowInfo != nullptr)
	{
		iPoint arrow_pos = arrowInfo->pointAt;
		if (arrowInfo->rotation == 0 || arrowInfo->rotation == 180)
			arrow_pos.x += ARROW_MOVEMENT * sin(App->tutorial->timer.ReadSec()* ARROW_SPEED);
		else
			arrow_pos.y += ARROW_MOVEMENT * sin(App->tutorial->timer.ReadSec()* ARROW_SPEED);

		App->tutorial->arrow->localPosition = arrow_pos;
		App->tutorial->arrow->rotation = arrowInfo->rotation;
		App->tutorial->arrow->BlitElement();
	}
}

bool Step::isFinished()
{
	bool ret = false;

	if (duration != -1 && App->tutorial->timer.Read() >= duration)
		ret = true;

	return ret;
}
