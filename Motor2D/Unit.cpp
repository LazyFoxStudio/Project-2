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

//minimap_
#include "j1UIScene.h"

#define COLLIDER_MARGIN 10  // extra margin for separation calculations  // 10 ~ 30//
#define MAX_NEXT_STEP_MODULE 50.0f   // max value for the next_step vector, for steering calculations  // 10 ~ 50//

#define SEPARATION_STRENGTH 1.25f   // the higher the stronger   // 1.0f ~ 10.0f//
#define SPEED_CONSTANT 2.5f   // applied to all units            // 60 ~ 140 //
#define STOP_TRESHOLD 0.3f										// 0.5f ~ 1.5f//

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

	current_anim = animations[1];

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

	if(dir == W  || dir == NW || dir == SW)
		App->render->Blit(texture, position.x - (r.w / 2), position.y - (r.h / 2), &r, true,SDL_FLIP_HORIZONTAL);
	else
		App->render->Blit(texture, position.x - (r.w / 2), position.y - (r.h / 2), &r);
	
}

bool Unit::Update(float dt)
{
	if (current_HP <= 0) return false;

	if (!commands.empty())
	{
		commands.front()->Execute(dt);
		if (commands.front()->state == FINISHED) commands.pop_front();
	}

	//minimap_
	if (App->uiscene->minimap != nullptr)
	{
		SDL_Color color;
		color.a = 255;
		if (type < ALLY_X)
		{
			color.r = 0;
			color.b = 255;
			color.g = 0;
		}
		else
		{
			color.r = 255;
			color.b = 0;
			color.g = 0;
		}
		App->uiscene->minimap->Addpoint({ (int)position.x,(int)position.y,50,50 }, color);
	}


	Move(dt);
	animationController();

	return true;
}

void Unit::Move(float dt)
{
	fPoint separation_v = calculateSeparationVector() * STEERING_FACTOR;

	if ((!commands.empty() ? commands.front()->type != ATTACK : false) || separation_v.GetModule() > STOP_TRESHOLD)
	{
		next_step = next_step + (separation_v * STEERING_FACTOR);

		if (next_step.GetModule() > MAX_NEXT_STEP_MODULE)
			next_step = next_step.Normalized() * MAX_NEXT_STEP_MODULE;

		fPoint last_pos = position;

		if (squad)	position += (next_step * squad->max_speed * dt * SPEED_CONSTANT);
		else		position += (next_step * speed * dt * SPEED_CONSTANT);

		if (!App->pathfinding->IsWalkable(App->map->WorldToMap(position.x, position.y))) 
		{ 
			iPoint unwalkable_tile = App->map->WorldToMap(position.x, position.y);
			iPoint unwalkable_tile_w = App->map->MapToWorld(unwalkable_tile.x, unwalkable_tile.y);
			SDL_Rect r = { unwalkable_tile_w.x, unwalkable_tile_w.y, App->map->data.tile_width, App->map->data.tile_height };
			SDL_Rect result = { 0,0,0,0 };
			SDL_IntersectRect(&collider, &r, &result);

			int x = (collider.x >= result.x ? result.w + 4 - (collider.w / 2) : -result.w - 4 + (collider.w / 2));
			int y = (collider.y >= result.y ? result.h + 4 - (collider.h / 2) : -result.h - 4 + (collider.h / 2));
			position += fPoint(x, y);
		}

		collider.x = position.x - (collider.w / 2);
		collider.y = position.y - (collider.h / 2);
	}
	else if (!next_step.IsZero()) next_step.SetToZero();
}

void Unit::setDirection()
{
	if (next_step.x != 0 || next_step.y != 0)
	{
		if (next_step.x < -MAX_NEXT_STEP_MODULE / 2)
		{
			if (next_step.y < MAX_NEXT_STEP_MODULE / 4 && next_step.y > -MAX_NEXT_STEP_MODULE / 4) dir = W;
			else dir = (next_step.y < 0 ? NW : SW);
		}
		else if (next_step.x > MAX_NEXT_STEP_MODULE / 2)
		{
			if (next_step.y < MAX_NEXT_STEP_MODULE / 4 && next_step.y > -MAX_NEXT_STEP_MODULE / 4) dir = E;
			else dir = (next_step.y < 0 ? NE : SE);
		}
		else dir = (next_step.y < 0 ? N : S);
	}
}

void Unit::animationController()
{
	setDirection();

	if (!next_step.IsZero())
	{
		switch (commands.empty() ? MOVETO : commands.front()->type)
		{
		case MOVETO || ATTACKING_MOVETO:
			switch (dir)
			{
			case E:  new_animation = MOVE_E;  break;
			case SE: new_animation = MOVE_SE; break;
			case NE: new_animation = MOVE_NE; break;
			case N:  new_animation = MOVE_N;  break;
			case S:  new_animation = MOVE_S;  break;
			case W:  new_animation = MOVE_W;  break;
			case NW: new_animation = MOVE_NW; break;
			case SW: new_animation = MOVE_SW; break;
			}
			break;
		case ATTACK:
			switch (dir)
			{
			case E:  new_animation = ATK_E;  break;
			case SE: new_animation = ATK_SE; break;
			case NE: new_animation = ATK_NE; break;
			case N:  new_animation = ATK_N;  break;
			case S:  new_animation = ATK_S;  break;
			case W:  new_animation = ATK_W;  break;
			case NW: new_animation = ATK_NW; break;
			case SW: new_animation = ATK_SW; break;
			}
			break;
		}
	}
	else if (commands.empty() ? false : commands.front()->type == ATTACK)
	{
		switch (dir)
		{
		case E:  new_animation = ATK_E;  break;
		case SE: new_animation = ATK_SE; break;
		case NE: new_animation = ATK_NE; break;
		case N:  new_animation = ATK_N;  break;
		case S:  new_animation = ATK_S;  break;
		case W:  new_animation = ATK_W;  break;
		case NW: new_animation = ATK_NW; break;
		case SW: new_animation = ATK_SW; break;
		}
	}
	else
	{
		switch (dir)
		{
		case E:  new_animation = IDLE_E;  break;
		case SE: new_animation = IDLE_SE; break;
		case NE: new_animation = IDLE_NE; break;
		case N:  new_animation = IDLE_N;  break;
		case S:  new_animation = IDLE_S;  break;
		case W:  new_animation = IDLE_W;  break;
		case NW: new_animation = IDLE_NW; break;
		case SW: new_animation = IDLE_SW; break;
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
		if (collisions[i]->entity_type == UNIT) {
			if (((Unit*)collisions[i])->IsEnemy() != IsEnemy()) continue;
		}
		else if (collisions[i]->entity_type == BUILDING && IsEnemy()) continue;

		fPoint current_separation = (position - collisions[i]->position);
		separation_v += (position - collisions[i]->position).Normalized() * (1 / current_separation.GetModule());
	}

	if (commands.empty() ? false : (commands.front()->type == ATTACKING_MOVETO || commands.front()->type == ATTACK))
		return separation_v * SEPARATION_STRENGTH * 0.3f;
	else
		return separation_v * SEPARATION_STRENGTH;
}