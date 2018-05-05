#include "j1Tutorial.h"
#include "UI_TextBox.h"
#include "UI_Image.h"
#include "UI_element.h"
#include "j1App.h"
#include "j1Gui.h"
//---- TEMP
#include "j1Input.h"

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

	Step* step = new Step();
	step->duration = -1;
	TextBox* text =  new TextBox(100, 100, 0, 0);
	text->addTextLine("Hola");
	text->addTextLine("adeu");
	step->text = text;
	ArrowInfo* info = new ArrowInfo();
	info->pointAt = { 150, 200 };
	info->rotation = 90;
	step->arrowInfo = info;
	steps.push_back(step);

	return true;
}

bool j1Tutorial::Update(float dt)
{
	if (doingTutorial)
	{
		if (missing_steps.size() == 0)
		{
			doingTutorial = false;
			active = false;
			return true;
		}
		for (std::list<Step*>::iterator it_s = missing_steps.begin(); it_s != missing_steps.end(); it_s++)
		{
			(*it_s)->Draw();
			if ((*it_s)->isFinished())
			{
				missing_steps.erase(it_s);
				timer.Start();
			}
		}
	}

	return true;
}

void j1Tutorial::loadTutorial(const char* path)
{

}

void j1Tutorial::startTutorial()
{
	if (!doingTutorial)
	{
		missing_steps = steps;
		doingTutorial = true;
		timer.Start();
	}
}

void j1Tutorial::finishStep()
{
	if (missing_steps.size() > 0)
	{
		missing_steps.erase(missing_steps.begin());
		timer.Start();
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
