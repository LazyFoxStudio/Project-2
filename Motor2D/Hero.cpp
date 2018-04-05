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
	
	if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_REPEAT)
	{
		//TODO pot ser que skill_one sigui nullptr i fa crash
		skill_one->Activate(this);
		LOG("skill one");
	}
	if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_UP)
	{
		skill_one->ready = true;
		LOG("restart");
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
