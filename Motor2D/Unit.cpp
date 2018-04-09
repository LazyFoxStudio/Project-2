#include "Unit.h"
#include "Hero.h"
#include "Effects.h"
#include "Squad.h"
#include "j1Pathfinding.h"
#include "j1Render.h"
#include "Command.h"
#include "p2Animation.h"
#include "j1EntityController.h"
#include "Skills.h"

#define COLLIDER_MARGIN 20  // extra margin for separation calculations  // 10 ~ 30//
#define MAX_NEXT_STEP_MODULE 20   // max value for the next_step vector, for steering calculations  // 10 ~ 50//

#define SEPARATION_STRENGTH 4.0f   // the higher the stronger   // 1.0f ~ 10.0f//
#define SPEED_CONSTANT 100   // applied to all units            // 60 ~ 140 //
#define STOP_TRESHOLD 1.0f										// 0.5f ~ 1.5f//

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
	squad_members			= unit.squad_members;

	for (int i = 0; i < unit.animations.size(); i++)
		animations.push_back(new Animation(*unit.animations[i]));

	current_anim = animations[0];

	for(int i = 0; i < 9; i++)
	 available_actions[i] = unit.available_actions[i];

	position.x = pos.x; position.y = pos.y;

	collider.x = pos.x - (collider.w / 2);
	collider.y = pos.y - (collider.h / 2);
}

Unit::~Unit()
{
	Halt();
	animations.clear();
	effects.clear();
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
	
	Move(dt);
	animationController();

	return true;
}

void Unit::Move(float dt)
{
	fPoint separation_v = calculateSeparationVector() * STEERING_FACTOR;

	if (!commands.empty() || separation_v.GetModule() > STOP_TRESHOLD)
	{
		if(commands.empty() || commands.front()->type != ATTACK)
			next_step = next_step + (separation_v * STEERING_FACTOR);

		if (next_step.GetModule() > MAX_NEXT_STEP_MODULE)
			next_step = next_step.Normalized() * MAX_NEXT_STEP_MODULE;

		fPoint last_pos = position;

		if (squad)	position += (next_step.Normalized() * squad->max_speed * dt * SPEED_CONSTANT);
		else		position += (next_step.Normalized() * speed * dt * SPEED_CONSTANT);

		if (!App->pathfinding->IsWalkable(App->map->WorldToMap(position.x, position.y))) position = last_pos;
		else
		{
			collider.x = position.x - (collider.w / 2);
			collider.y = position.y - (collider.h / 2);
		}
	}
	else if (!next_step.IsZero()) next_step.SetToZero();
}

void Unit::animationController()
{
	if (type == HERO_1) return;

	if (!next_step.IsZero())
	{
		switch (commands.empty() ? MOVETO : commands.front()->type)
		{
		case MOVETO:
			if (next_step.x > 0 && next_step.y < MAX_NEXT_STEP_MODULE/2 && next_step.y > -MAX_NEXT_STEP_MODULE/2) //MOVE E
			{
				new_animation = MOVE_E;
			}
			else if (next_step.x < 0 && next_step.y < MAX_NEXT_STEP_MODULE/2 && next_step.y > -MAX_NEXT_STEP_MODULE/2) //MOVE W
			{
				new_animation = MOVE_W;
			}
			else if (next_step.y > 0) //MOVE S
			{
				if (next_step.x < 3 && next_step.x > -3)
				{
					new_animation = MOVE_S;
				}
				else if (next_step.x < MAX_NEXT_STEP_MODULE/2)
				{
					new_animation = MOVE_SE;
				}
				else if (next_step.x < -MAX_NEXT_STEP_MODULE/2)
				{
					new_animation = MOVE_SW;
				}
			}
			else if (next_step.y < 0) //MOVE N
			{
				if (next_step.x < 3 && next_step.x > -3)
				{
					new_animation = MOVE_N;
				}
				if (next_step.x > MAX_NEXT_STEP_MODULE/2)
				{
					new_animation = MOVE_NW;
				}
				else if (next_step.x < -MAX_NEXT_STEP_MODULE/2)
				{
					new_animation = MOVE_NE;
				}				
			}
			break; //just in case...
		case ATTACK:
			break;
		}
	}
	else {
		switch (new_animation)
		{
		case MOVE_N:
			new_animation = IDLE_N;
			break;
		case MOVE_NE:
			new_animation = IDLE_NE;
			break;
		case MOVE_E:
			new_animation = IDLE_E;
			break;
		case MOVE_SE:
			new_animation = IDLE_SE;
			break;
		case MOVE_S:
			new_animation = IDLE_S;
			break;
		case MOVE_SW:
			new_animation = IDLE_SE;
			break;
		case MOVE_W:
			new_animation = IDLE_W;
			break;
		case MOVE_NW:
			new_animation = IDLE_NW;
			break;
		}
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


fPoint Unit::calculateSeparationVector()
{
	SDL_Rect r = { collider.x - COLLIDER_MARGIN, collider.y - COLLIDER_MARGIN , collider.w + COLLIDER_MARGIN , collider.h + COLLIDER_MARGIN };
	std::vector<Entity*> collisions = App->entitycontroller->CheckCollidingWith(r, this);

	fPoint separation_v = { 0,0 };
	for (int i = 0; i < collisions.size(); i++)
	{
		fPoint current_separation = (position - collisions[i]->position);
		separation_v += current_separation.Normalized() * (1 / current_separation.GetModule());
	}

	return separation_v * SEPARATION_STRENGTH;
}