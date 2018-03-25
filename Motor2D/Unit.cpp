#include "Unit.h"
#include "Effects.h"
#include "Squad.h"
#include "j1Render.h"
#include "Command.h"
#include "p2Animation.h"

Unit::Unit(iPoint pos, Unit& unit, Squad* squad) : squad(squad)
{
	name					= unit.name;
	texture					= unit.texture;
	type					= unit.type;
	flying					= unit.flying;
	animations				= unit.animations;

	attack					= unit.attack;
	current_HP = max_HP		= unit.max_HP;
	defense					= unit.defense;
	piercing_atk			= unit.piercing_atk;
	speed					= unit.speed;
	line_of_sight			= unit.line_of_sight;
	range					= unit.range;

	current_anim			= animations[0];
	entity_type				= UNIT;
	position.x = pos.x, position.y = pos.y;
}

void Unit::Draw(float dt)
{
	App->render->Blit(texture, position.x, position.y, &current_anim->GetCurrentFrame(dt));
}


void Unit::animationController()
{

}

void Unit::Halt()
{
	for (std::deque<Command*>::iterator it = commands.begin(); it != commands.end(); it++)
		(*it)->Restart();  // Restarting the order calls onStop(), which would be otherwise unaccesible

	commands.clear();
}

Unit* Unit::SearchNearestEnemy()
{
	//TODO
	return nullptr;
}