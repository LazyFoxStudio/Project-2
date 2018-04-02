#include "Unit.h"
#include "Hero.h"
#include "Effects.h"
#include "Squad.h"
#include "j1Pathfinding.h"
#include "j1Render.h"
#include "Command.h"
#include "p2Animation.h"
#include "Skills.h"

Unit::Unit(iPoint pos, Unit& unit, Squad* squad) : squad(squad)
{
	name					= unit.name;
	texture					= unit.texture;
	type					= unit.type;
	flying					= unit.flying;
	collider				= unit.collider;

	attack					= unit.attack;
	current_HP = max_HP		= unit.max_HP;
	defense					= unit.defense;
	piercing_atk			= unit.piercing_atk;
	speed					= unit.speed;
	line_of_sight			= unit.line_of_sight;
	range					= unit.range;

	for (int i = 0; i < unit.animations.size(); i++)
		animations.push_back(new Animation(*unit.animations[i]));

	current_anim = animations[0];

	entity_type				= UNIT;

	position.x = pos.x, position.y = pos.y;

	collider.w = current_anim->frames[0].w;
	collider.h = current_anim->frames[0].h;
	collider.x = position.x, collider.y = position.y;

	if (type < HERO_X)
	{
		//TODO check why it returns a different data instead of the assigned
		//((Hero*)(this))->skill_one= ((Hero*)(&unit))->skill_one;
	}
}

void Unit::Draw(float dt)
{
	if (type < HERO_X)
	{
		//blit hero sprites
		App->render->DrawQuad({ (int)position.x,(int)position.y,26,36 }, Blue, true);
	}
	else
	{
		App->render->Blit(texture, position.x, position.y, &current_anim->GetCurrentFrame(dt));
	}
}

bool Unit::Update(float dt)
{
	if (!commands.empty())
	{
		commands.front()->Execute(dt);
		if (commands.front()->state == FINISHED) commands.pop_front();
	}

	if (this->type == HERO_1)
	{
		((Hero*)this)->Hero::Update(dt);
	}
	animationController();
	
	return true;
}

void Unit::animationController()
{
	if (commands.empty())
	{
		new_animation = IDLE;
	}
	else
	{
		switch (commands.front()->type)
		{
		case MOVETO:
			MoveTo* move_command = (MoveTo*)commands.front();
			if (move_command->next_step.x > 0)
			{
				if (move_command->next_step.y < 0) //NOTH-EAST
				{
					
				}
				if (move_command->next_step.y == 0) //EAST
				{
					animations[5]->Reset();
					animations[4]->Reset();
					animations[9]->Reset();
					current_anim = animations[7];
				}
				if (move_command->next_step.y > 0) //SOUTH-EAST
				{

				}
			}
			if (move_command->next_step.x < 0)
			{
				if (move_command->next_step.y < 0) //NORTH-WEST
				{

				}
				if (move_command->next_step.y == 0) //WEST
				{

				}
				if (move_command->next_step.y > 0) //SOUTH-WEST
				{

				}
			}
			if (move_command->next_step.x == 0)
			{
				if (move_command->next_step.y < 0) //NORTH
				{
					animations[4]->Reset();
					animations[7]->Reset();
					animations[9]->Reset();
					current_anim = animations[5];
				}
				if (move_command->next_step.y == 0) //IDLE
				{
					animations[5]->Reset();
					animations[7]->Reset();
					animations[9]->Reset();
					current_anim = animations[4];
				}
				if (move_command->next_step.y > 0) //SOUTH
				{
					animations[5]->Reset();
					animations[7]->Reset();
					animations[4]->Reset();
					current_anim = animations[9];
				}
			}
			break;
		}

	}
	
}

void Unit::Halt()
{
	for (std::deque<Command*>::iterator it = commands.begin(); it != commands.end(); it++)
		(*it)->Restart();  // Restarting the order calls onStop(), which would be otherwise unaccesible

	commands.clear();
}

bool Unit::Pushed(fPoint direction)
{
	iPoint perpendicular_A = { 0,0 };
	iPoint new_dest = { 0,0 };

	if (direction.x == 0) perpendicular_A.x = 1;
	else				  perpendicular_A.y = 1;

	iPoint perpendicular_B = { -perpendicular_A.x, -perpendicular_A.y };
	iPoint map_pos = App->map->WorldToMap(position.x, position.y);

	if (App->pathfinding->IsWalkable(map_pos + perpendicular_A))		new_dest = map_pos + perpendicular_A;
	else if(App->pathfinding->IsWalkable(map_pos + perpendicular_B))	new_dest = map_pos + perpendicular_B;
	else																return false;

	commands.push_front(new MoveTo(this, new_dest));
	return true;
}

Unit* Unit::SearchNearestEnemy()
{
	//TODO
	return nullptr;
}