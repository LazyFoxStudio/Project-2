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
	entity_type				= UNIT;
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

	position.x =  pos.x; position.y = pos.y;

	collider.x = pos.x - (collider.w / 2);
	collider.y = pos.y - (collider.h / 2);
}

void Unit::Draw(float dt)
{
	SDL_Rect r = current_anim->GetCurrentFrame(dt);

	if (new_animation == MOVE_W || new_animation == IDLE_W)
	{
		App->render->Blit(texture, position.x - (r.w / 2), position.y - (r.h / 2), &r, true,SDL_FLIP_HORIZONTAL);
	}
	else
	{
		App->render->Blit(texture, position.x - (r.w / 2), position.y - (r.h / 2), &r);
	}
	
}

bool Unit::Update(float dt)
{
	if (!commands.empty())
	{
		commands.front()->Execute(dt);
		if (commands.front()->state == FINISHED) commands.pop_front();
	}

	animationController();
	return true;
}

void Unit::animationController()
{
	if (type == HERO_1) return;

	if (!next_step.IsZero())
	{
		switch (commands.front()->type)
		{
		case MOVETO:
			if (next_step.x > 0) //MOVE E
				new_animation = MOVE_E;
			else if (next_step.x < 0) //MOVE W
				new_animation = MOVE_W;
			else if (next_step.y > 0) //MOVE S
				new_animation = MOVE_S;
			else if (next_step.y < 0) //MOVE N
				new_animation = MOVE_N;
			break; //just in case...
		case ATTACK:
			break;
		}
	}
	else {
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
		new_animation = IDLE_S;
	}

	if (animations[new_animation] != current_anim)
	{
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

bool Unit::Pushed()
{
	if (!commands.empty())
	{
		if (commands.front()->type == MOVETO)
		{
			iPoint map_p = App->map->WorldToMap(position.x, position.y);
			MoveTo* current_moveto_order = (MoveTo*)commands.front();
			iPoint target = current_moveto_order->flow_field->getNodeAt(map_p)->parent->position;

			iPoint new_target = App->pathfinding->WalkableAdjacentCloserTo(map_p, target, this);
			if (new_target.x == -1) return false;

			current_moveto_order->flow_field->getNodeAt(map_p)->parent = current_moveto_order->flow_field->getNodeAt(target);
			return true;
		}
	}
		
	return false;
}

Unit* Unit::SearchNearestEnemy()
{
	//TODO
	return nullptr;
}