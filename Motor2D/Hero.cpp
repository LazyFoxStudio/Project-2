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

	if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_DOWN)			current_skill != 1 ? current_skill = 1 : current_skill = 0;
	else if (App->input->GetKey(SDL_SCANCODE_W) == KEY_DOWN)	current_skill != 2 ? current_skill = 2 : current_skill = 0;
	else if (App->input->GetKey(SDL_SCANCODE_E) == KEY_DOWN)	current_skill != 3 ? current_skill = 3 : current_skill = 0;

	switch (current_skill)
	{
	case 1: 
		skill_one->DrawRange(); 
		if (App->input->GetMouseButtonDown(1) == KEY_DOWN && skill_one->Ready())  skill_one->Activate();
		break;
	case 2:
		skill_two->DrawRange();
		if (App->input->GetMouseButtonDown(1) == KEY_DOWN && skill_two->Ready())  skill_two->Activate();
		break;
	case 3:
		skill_three->DrawRange();
		if (App->input->GetMouseButtonDown(1) == KEY_DOWN && skill_three->Ready())  skill_three->Activate();
		break;
	default: 
		break;
	}

	if (!commands.empty())
	{
		commands.front()->Execute(dt);
		if (commands.front()->state == FINISHED) commands.pop_front();
	}

	Move(dt);

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
