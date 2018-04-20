#include "Hero.h"
#include "j1Render.h"
#include "j1Input.h"
#include "Color.h"
#include "Command.h"
#include "j1EntityController.h"
#include "Squad.h"
#include "j1Gui.h"
#include "j1Audio.h"

#include "j1UIScene.h"

Hero::~Hero()
{
	RELEASE(skill_one);
	RELEASE(skill_two);
	RELEASE(skill_three);
}

bool Hero::Update(float dt)
{
	if (current_HP <= 0)
	{
		if (isActive) Deactivate();
		else if (revive_timer.ReadSec() > HERO_REVIVE_COOLDOWN) { current_HP = max_HP;  setActive(true); App->gui->createLifeBar(this); }
		return true;
	}

	switch (current_skill)
	{
	case 1: 
		skill_one->DrawRange(); 
		if (App->input->GetMouseButtonDown(1) == KEY_DOWN && skill_one->Ready())  skill_one->Activate(), App->audio->PlayFx(SFX_HERO_YHAMAM_ICICLECRASH);
		skill_one->toDraw.clear();
		break;
	case 2:
		skill_two->DrawRange();
		if (App->input->GetMouseButtonDown(1) == KEY_DOWN && skill_two->Ready())  skill_two->Activate(), App->audio->PlayFx(SFX_HERO_YHAMAM_OVERFLOW);
		skill_two->toDraw.clear();		
		break;
	case 3:
		skill_three->DrawRange();
		if (App->input->GetMouseButtonDown(1) == KEY_DOWN && skill_three->Ready())  skill_three->Activate(), App->audio->PlayFx(SFX_HERO_YHAMAM_DRAGONBREATH);
		skill_three->toDraw.clear();	
		break;
	default: 
		break;
	}

	if (App->input->GetMouseButtonDown(3) == KEY_DOWN && current_skill!=0)  current_skill=0;

	if (!commands.empty())
	{
		commands.front()->Execute(dt);
		if (commands.front()->state == FINISHED) commands.pop_front();
	}

	Move(dt);
	animationController();

	//minimap_
	if (App->uiscene->minimap)
		App->uiscene->minimap->Addpoint({ (int)position.x,(int)position.y,75,75 }, White);

	return true;
}

void Hero::Deactivate()
{
	setActive(false);
	revive_timer.Start();
	App->entitycontroller->selected_squads.remove(squad);
	App->entitycontroller->selected_entities.remove(this);
	App->gui->entityDeleted(this);
	current_skill = 0;
	isSelected = false;
}
