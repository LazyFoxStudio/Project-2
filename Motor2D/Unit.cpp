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
	}
}

void Unit::Draw(float dt)
{
	
	if (new_animation == MOVE_W || new_animation == IDLE_W)
	{
		App->render->Blit(texture, position.x, position.y, &current_anim->GetCurrentFrame(dt),true,SDL_FLIP_HORIZONTAL);
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
		if (commands.front()->state == FINISHED) 
		{
			commands.pop_front();
		}
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
	if (!commands.empty())
	{
		MoveTo* move_command = nullptr;
		switch (commands.front()->type)
		{
		case MOVETO:
			move_command = (MoveTo*)commands.front();
			if (move_command->next_step.x > 0) //MOVE E
			{
				new_animation = MOVE_E;
				break;
			}
			if (move_command->next_step.x < 0) //MOVE W
			{
				new_animation = MOVE_W;
				break;
			}
			if (move_command->next_step.y > 0) //MOVE S
			{
				new_animation = MOVE_S;
				break;
			}
			if (move_command->next_step.y < 0) //MOVE N
			{
				new_animation = MOVE_N;
				break;
			}
			if (move_command->next_step.y + move_command->next_step.x == 0)
			{
				/*
				switch (new_animation)
				{
				case MOVE_E:
					new_animation = IDLE_E;
					break;
				case MOVE_N:
					new_animation = IDLE_N;
					break;
				case MOVE_S:
					new_animation = IDLE_S;
					break;
				case MOVE_W:
					new_animation = IDLE_W;
				}
				*/
				new_animation = MOVE_E;
			}
			break; //just in case...
		case ATTACK:
			break;
		}
	}

	if (animations[new_animation] != current_anim && type != HERO_1)
	{
		current_anim->Reset();
		current_anim = animations[new_animation];
		current_anim->Reset();
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