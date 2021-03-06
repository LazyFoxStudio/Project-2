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
#include "j1EntityController.h"
#include "UI_Button.h"
#include "UI_UnlockDisplay.h"

j1Tutorial::j1Tutorial()
{
}


j1Tutorial::~j1Tutorial()
{
}

bool j1Tutorial::Start()
{
	name = "tutorial";
	pausable = false;

	loadTutorial("tutorial.xml");

	arrow = new Image(App->gui->atlas, 0, 0, { 991, 809, 149, 113 }, nullptr);
	arrow->use_camera = true;

	return true;
}

bool j1Tutorial::PreUpdate()
{
	if (doingTutorial && activeStep != nullptr && !activeStep->finished && activeStep->task == KILL_ENEMIES)
	{
		bool enemies_found = false;
		for (std::list<Entity*>::iterator it_e = App->entitycontroller->operative_entities.begin(); it_e != App->entitycontroller->operative_entities.end(); it_e++)
		{
			if ((*it_e)->IsEnemy())
			{
				enemies_found = true;
				break;
			}
		}
		if (!enemies_found)
			taskCompleted(KILL_ENEMIES);
	}

	return true;
}

bool j1Tutorial::Update(float dt)
{
	if (doingTutorial)
	{
		if (missing_steps.size() == 0 && activeStep == nullptr)
		{
			tutorialDone = true;
			stopTutorial(true);
			return true;
		}
		if (activeStep == nullptr)
		{
			activeStep = missing_steps.front();
			stepStarted(activeStep->task);
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
		{
			activeStep = nullptr;
		}
		else if (activeStep->finished)
		{
			if (completed_delay_timer.Read() >= completed_delay)
			{		
				activeStep = nullptr;
			}
		}
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
		for (std::list<Step*>::iterator it_s = missing_steps.begin(); it_s != missing_steps.end(); it_s++)
		{
			(*it_s)->finished = false;
			(*it_s)->text->changeColor(DEFAULT_COLOR);
		}
		doingTutorial = true;
		App->wavecontroller->tutorial = true;
		timer.Start();
		allowTHSelection = false;
		allowHeroSelection = false;
		enemiesKilled = false;
		allowTHSelection = false;
		allowHeroSelection = false;
		App->gui->nextWaveWindow->active = false;

		Button* barracks = App->gui->GetActionButton(5);
		barracks->setCondition("First finish the tutorial");
		barracks->Lock();
		Button* lumber = App->gui->GetActionButton(6);
		lumber->setCondition("First finish the tutorial");
		lumber->Lock();
		Button* farms = App->gui->GetActionButton(7);
		farms->setCondition("First finish the tutorial");
		farms->Lock();
		Button* mine = App->gui->GetActionButton(22);
		mine->setCondition("First finish the tutorial");
		mine->Lock();
		Button* turret = App->gui->GetActionButton(23);
		turret->setCondition("First finish the tutorial");
		turret->Lock();
		Button* hut = App->gui->GetActionButton(24);
		hut->setCondition("First finish the tutorial");
		hut->Lock();
		Button* church = App->gui->GetActionButton(25);
		church->setCondition("First finish the tutorial");
		church->Lock();
		Button* blacksmith = App->gui->GetActionButton(26);
		blacksmith->setCondition("First finish the tutorial");
		blacksmith->Lock();
	}
}

void j1Tutorial::stopTutorial(bool skip)
{
	if (doingTutorial)
	{
		missing_steps.clear();
		activeStep = nullptr;
		doingTutorial = false;
		App->wavecontroller->wave_timer.Start();
		App->gui->nextWaveWindow->timer->counter.Start();
		App->wavecontroller->tutorial = false;

		if (skip)
			active = false;

		Button* lumber = App->gui->GetActionButton(6);
		lumber->Unlock();
		Button* barracks = App->gui->GetActionButton(5);
		Button* farms = App->gui->GetActionButton(7);
		Button* mine = App->gui->GetActionButton(22);
		Button* hut = App->gui->GetActionButton(24);
		Button* turret = App->gui->GetActionButton(23);
		if (!skip || tutorialDone)
		{			
			App->gui->unlockDisplay->unlockedBuilding(lumber->section, "Lumber Mill");
			barracks->Unlock();		
			App->gui->unlockDisplay->unlockedBuilding(barracks->section, "Barracks");
			farms->Unlock();
			App->gui->unlockDisplay->unlockedBuilding(farms->section, "Farm");
			mine->Unlock();
			App->gui->unlockDisplay->unlockedBuilding(mine->section, "Mine");
			hut->Unlock();
			App->gui->unlockDisplay->unlockedBuilding(hut->section, "Gnome Hut");
			turret->Unlock();
			App->gui->unlockDisplay->unlockedBuilding(turret->section, "Turret");
		}
		else
		{
			barracks->setCondition("Build first a Lumber Mill");
			farms->setCondition("Build first a Lumber Mill");
			mine->setCondition("Build first a Lumber Mill");
			hut->setCondition("Build first the Barracks");
			turret->setCondition("Build first a Farm");
		}
				
		Button* church = App->gui->GetActionButton(25);
		church->setCondition("Build first a Mine");
		church->Lock();
		Button* blacksmith = App->gui->GetActionButton(26);
		blacksmith->setCondition("Build first a Gnome Hut");
		blacksmith->Lock();

		App->gui->nextWaveWindow->active = true;
	}
}

void j1Tutorial::finishStep()
{
	if (activeStep != nullptr)
	{
		App->audio->PlayFx(SFX_TUTORIAL_STEP, 80);
		activeStep->finished = true;
		activeStep->text->changeColor(Translucid_Green);
		completed_delay_timer.Start();		
	}
}

void j1Tutorial::taskCompleted(Task task)
{
	if (activeStep != nullptr && activeStep->task == task && !activeStep->finished)
	{
		finishStep();

		if (task == PLACE_LUMBER_MILL)
		{
			Button* lumber = App->gui->GetActionButton(6);
			lumber->Lock();
		}
		else if (task == PLACE_BARRACKS)
		{
			Button* barracks = App->gui->GetActionButton(5);
			barracks->Lock();
		}
		else if (task == PLACE_FARM)
		{
			Button* farms = App->gui->GetActionButton(7);
			farms->Lock();
			App->wavecontroller->forceNextWave();
			App->gui->nextWaveWindow->timer->counter.PauseTimer();
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
		else if (task == MOVE_CAMERA)
		{
			allowTHSelection = true;
		}
	}
	else if (activeStep != nullptr && !activeStep->finished) //to avoid skiping steps and bugging the tutorial
	{
		if (task == KILL_ENEMIES)
			enemiesKilled = true;
		if (activeStep->task == SELECT_LUMBER_MILL && task == ASSIGN_WORKERS)
		{
			workersAssigned = true;
			finishStep();
		}
	}
}

void j1Tutorial::stepStarted(Task task)
{
	Button* lumber = nullptr;
	Button* farms = nullptr;
	Button* barracks = nullptr;

	switch (task)
	{
	case SELECT_TOWN_HALL:
		if (App->entitycontroller->selected_entities.size() > 0 && App->entitycontroller->selected_entities.front()->type == TOWN_HALL)
			taskCompleted(SELECT_TOWN_HALL);
		break;
	case PICK_LUMBER_MILL:
		lumber = App->gui->GetActionButton(6);
		lumber->Unlock();
		break;
	case PLACE_LUMBER_MILL:
		break;
	case PLACE_FARM:
		farms = App->gui->GetActionButton(7);
		farms->Unlock();
		break;
	case SELECT_HERO:
		allowHeroSelection = true;
		if (App->entitycontroller->selected_entities.size() > 0 && App->entitycontroller->selected_entities.front()->IsHero())
			taskCompleted(SELECT_HERO);
		break;
	case MOVE_TROOPS:
		break;
	case KILL_ENEMIES:
		if (enemiesKilled)
			taskCompleted(KILL_ENEMIES);
		break;
	case PLACE_BARRACKS:
		barracks = App->gui->GetActionButton(5);
		barracks->Unlock();
		break;
	case MOVE_CAMERA:
		break;
	case ASSIGN_WORKERS:
		if (workersAssigned)
			finishStep();
		break;
	default:
		break;
	}
}

void Step::Draw()
{
	if (text != nullptr)
		text->BlitElement();
	if (arrowInfo != nullptr)
	{
		iPoint arrow_pos = arrowInfo->pointAt;
		if (task == SELECT_HERO)
		{
			arrow_pos.x = App->entitycontroller->getEntitybyID(App->entitycontroller->hero_UID)->position.x - App->tutorial->arrow->section.w/2;
			arrow_pos.y = App->entitycontroller->getEntitybyID(App->entitycontroller->hero_UID)->position.y - App->tutorial->arrow->section.h - ARROW_MOVEMENT - 10;
			App->tutorial->arrow->use_camera = true;
		}
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
