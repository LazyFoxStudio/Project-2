#include "Hero.h"
#include "j1Render.h"
#include "j1Input.h"
#include "Color.h"
#include "Command.h"

Hero::Hero(iPoint pos, Hero& hero)
{
	
}
Hero::~Hero()
{
	RELEASE(skill_one);
}

bool Hero::Update(float dt)
{
	//if (type ==MAGE)
	//{
	//	//blit hero sprites
	//	App->render->DrawQuad({ (int)position.x,(int)position.y,26,36 }, Blue, true);
	//}
	
	if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_DOWN && (doSkill_1 || skill_one->ready))
	{
		doSkill_1 = !doSkill_1;

		if (doSkill_2 || doSkill_3)
		{
			doSkill_2 = false;
			doSkill_3 = false;
		}
	}
	
	if (App->input->GetKey(SDL_SCANCODE_W) == KEY_DOWN && (doSkill_2 || skill_two->ready))
	{
		doSkill_2 = !doSkill_2;

		if (doSkill_1 || doSkill_3)
		{
			doSkill_1 = false;
			doSkill_3 = false;
		}
	}

	if (App->input->GetKey(SDL_SCANCODE_E) == KEY_DOWN && (doSkill_3 || skill_three->ready))
	{
		doSkill_3 = !doSkill_3;

		if (doSkill_1 || doSkill_2)
		{
			doSkill_1 = false;
			doSkill_2 = false;
		}
	}

	if (doSkill_1 && skill_one->ready)
	{
		skill_one->Activate(this);
	}
	if (doSkill_2 && skill_two->ready)
	{
		skill_two->Activate(this);
	}
	if (doSkill_3 && skill_three->ready)
	{
		skill_three->Activate(this);
	}

	if (skill_one->timer.ReadSec()>= skill_one->cooldown && !skill_one->ready)
	{
		skill_one->ready = true;
	}
	if (skill_two->timer.ReadSec() >= skill_two->cooldown && !skill_two->ready)
	{
		skill_two->ready = true;
	}
	if (skill_three->timer.ReadSec() >= skill_three->cooldown && !skill_three->ready)
	{
		skill_three->ready = true;
	}
	
	if (!((Unit*)(this))->commands.empty())
	{
		((Unit*)(this))->commands.front()->Execute(dt);
		if (((Unit*)(this))->commands.front()->state == FINISHED) commands.pop_front();
	}

	((Unit*)(this))->Move(dt);

	return true;
}

void Hero::UseSkill(int index)
{

}

void Hero::attackEntity(Entity* target)
{

}

Entity* Hero::findTarget()
{
	return nullptr;
}
