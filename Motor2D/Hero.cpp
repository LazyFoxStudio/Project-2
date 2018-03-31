#include "Hero.h"
#include "j1Input.h"

Hero::~Hero()
{

}

bool Hero::Update(float dt)
{
	if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_REPEAT)
	{
		//TODO pot ser que skill_one sigui nullptr i fa crash
		//skill_one->Activate(this);
		LOG("skill one");
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
