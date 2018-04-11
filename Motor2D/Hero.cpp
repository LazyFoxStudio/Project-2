#include "Hero.h"
#include "j1Render.h"
#include "j1Input.h"
#include "Color.h"
#include "Command.h"

#include "j1UIScene.h"

Hero::Hero(iPoint pos, Hero& hero)
{
	
}
Hero::~Hero()
{
	RELEASE(skill_one);
}

bool Hero::Update(float dt)
{
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

	//minimap_
	if (App->uiscene->minimap != nullptr)
	{
		SDL_Color color;
		color.r = 255;
		color.b = 255;
		color.g = 255;
		color.a = 255;
		App->uiscene->minimap->Addpoint({ (int)position.x,(int)position.y,75,75 }, color);
	}


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
