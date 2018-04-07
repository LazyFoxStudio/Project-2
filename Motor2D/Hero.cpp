#include "Hero.h"
#include "j1Render.h"
#include "j1Input.h"
#include "Color.h"

Hero::Hero(iPoint pos, Hero& hero)
{
	name = hero.name;
	texture = hero.texture;
	collider = hero.collider;
	type = hero.type;


	attack = hero.attack;
	current_HP = max_HP = hero.max_HP;
	defense = hero.defense;
	piercing_atk = hero.piercing_atk;
	speed = hero.speed;
	line_of_sight = hero.line_of_sight;
	range = hero.range;

	for (int i = 0; i < hero.animations.size(); i++)
		animations.push_back(new Animation(*hero.animations[i]));

	current_anim = animations[0];

	entity_type = HERO;

	position.x = collider.x = pos.x;
	position.y = collider.y = pos.y;
	
	skill_one = hero.skill_one;
	skill_two = hero.skill_two;
}
Hero::~Hero()
{
	RELEASE(skill_one);
}

bool Hero::Update(float dt)
{
	if (type ==MAGE)
	{
		//blit hero sprites
		App->render->DrawQuad({ (int)position.x,(int)position.y,26,36 }, Blue, true);
	}
	
	if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_DOWN && (doSkill_1 || skill_one->ready))
	{
		doSkill_1 = !doSkill_1;

		if (doSkill_2)
		{
			doSkill_2 = false;
		}
	}
	
	if (App->input->GetKey(SDL_SCANCODE_W) == KEY_DOWN && (doSkill_2 || skill_two->ready))
	{
		doSkill_2 = !doSkill_2;

		if (doSkill_1)
		{
			doSkill_1 = false;
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

	if (skill_one->timer.ReadSec()>= skill_one->cooldown && !skill_one->ready)
	{
		skill_one->ready = true;
		LOG("---------------------------Skill_1 ready!!!");
	}
	if (!skill_one->ready)
	{
		LOG("seconds to restart %f", skill_one->timer.ReadSec());
	}

	if (skill_two->timer.ReadSec() >= skill_two->cooldown && !skill_two->ready)
	{
		skill_two->ready = true;
		LOG("---------------------------Skill_2 ready!!!");
	}
	if (!skill_two->ready)
	{
		LOG("seconds to restart %f", skill_two->timer.ReadSec());
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
