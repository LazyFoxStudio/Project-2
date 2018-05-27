#include "Unit.h"
#include "Hero.h"
#include "Effects.h"
#include "Squad.h"
#include "j1Pathfinding.h"
#include "j1Render.h"
#include "p2Animation.h"
#include "j1EntityController.h"
#include "Skills.h"
#include "j1Gui.h"
#include "Quadtree.h"
#include "Minimap.h"
#include "j1Tutorial.h"
    
#define MAX_SEPARATION_WEIGHT 2.0f   

Unit::Unit(iPoint pos, Unit& unit, Squad* squad) : squad(squad)
{
	texture					= unit.texture;
	type					= unit.type;
	flying					= unit.flying;
	collider				= unit.collider;
	cost					= unit.cost;

	attack					= unit.attack;
	current_HP = max_HP		= unit.max_HP;
	defense					= unit.defense;
	piercing_atk			= unit.piercing_atk;
	speed					= unit.speed;
	line_of_sight			= unit.line_of_sight;
	range					= unit.range;

	for (int i = 0; i < unit.animations.size(); i++)
		animations.push_back(new Animation(*unit.animations[i]));

	current_anim = animations[IDLE_S];

	for(int i = 0; i < 9; i++)
	 available_actions = unit.available_actions;

	infoData = unit.infoData;

	position.x = pos.x; position.y = pos.y;
	mov_target = position;

	collider.x = pos.x - (collider.w / 2);
	collider.y = pos.y - (collider.h / 2);



}

Unit::~Unit()
{
	for (int i = 0; i < animations.size(); i++)
		RELEASE(animations[i]);

	animations.clear();
	effects.clear();
}

void Unit::Draw(float dt)
{
	animationController();

	anim = current_anim->GetCurrentFrame(dt);

	if (App->render->CullingCam(position))
	{
		App->entitycontroller->SpriteQueue.push(this);
	}
}

bool Unit::Update(float dt)
{
	if (current_HP > 0)
	{
		//take buffs out here
		for (std::list<Effect*>::iterator it = effects.begin(); it != effects.end(); it++)
		{
			if ((*it)->applied)
			{
				(*it)->Remove();
			}
		}

		//remove buffs here
		for (std::list<Effect*>::iterator it = effects.begin(); it != effects.end(); it++)
		{
			float time_passed = (*it)->timer.ReadSec();
			float max_time = (*it)->duration;
			if ((int)time_passed > (int)max_time)
			{
				Effect* to_del = (*it);
				effects.remove(*it);
				RELEASE(to_del);
			}
		}

		//apply buffs here
		for (std::list<Effect*>::iterator it = effects.begin(); it != effects.end(); it++)
		{
			(*it)->Apply();
			(*it)->applied = true;
		}

		if (!commands.empty())
		{
			commands.front()->Execute(dt);
			if (commands.front()->state == FINISHED) commands.pop_front();
		}
		else if (squad->commander_pos + squad->getOffset(UID) != mov_target) mov_target = squad->commander_pos + squad->getOffset(UID);

		Move(dt);

		//minimap
		if (IsEnemy())
			App->gui->minimap->Addpoint({ (int)position.x,(int)position.y,50,50 }, Red);
		else
			App->gui->minimap->Addpoint({ (int)position.x,(int)position.y,50,50 }, Green);

		return true;
	}
	else
		return false;
}



void Unit::Move(float dt)
{
	float max_step = dt * speed * SPEED_CONSTANT;

	fPoint separation_v = { 0.0f, 0.0f };
	float separation_w = 0.0f;

	calculateSeparationVector(separation_v, separation_w);

	if ((mov_target - position).GetModule() > 0 && (!commands.empty() ? commands.front()->type != ATTACK : false))
	{
		if (getCurrentCommand() != NOTHING || separation_w == 0)
		{
			mov_direction = (mov_target - position);

			if (mov_direction.GetModule() < max_step)
				mov_module = mov_direction.GetModule() / max_step;
			else
				mov_module = MAX_NEXT_STEP_MULTIPLIER;

			mov_direction.Normalize();
		}
	}
	else
	{
		mov_target = position;
		mov_module = 0;
		separation_w *= 2;
	}

	fPoint movement = (((mov_direction * mov_module) + (separation_v * separation_w)) * max_step);

	if (movement.GetModule() > max_step * MAX_NEXT_STEP_MULTIPLIER)
		movement = movement.Normalized() * max_step * MAX_NEXT_STEP_MULTIPLIER;

	if (App->pathfinding->IsWalkable(App->map->WorldToMap(position.x + movement.x, position.y + movement.y)) || IsFlying())
		position += movement;
	else
		position -= movement * 0.3f;

	collider.x = position.x - (collider.w / 2);
	collider.y = position.y - (collider.h / 2);
	
}



void Unit::lookAt(fPoint direction)
{
	float v_module = direction.GetModule();
	if (direction.x != 0 || direction.y != 0)
	{
		if (direction.x < -v_module / 2)
		{
			if (direction.y < v_module / 4 && direction.y > -v_module / 4) dir = W;
			else dir = (direction.y < 0 ? NW : SW);
		}
		else if (direction.x > v_module / 2)
		{
			if (direction.y < v_module / 4 && direction.y > -v_module / 4) dir = E;
			else dir = (direction.y < 0 ? NE : SE);
		}
		else dir = (direction.y < 0 ? N : S);
	}
}


void Unit::Halt()
{
	for (std::deque<Command*>::iterator it = commands.begin(); it != commands.end(); it++)
		(*it)->Restart();  // Restarting the order calls onStop(), which would be otherwise unaccesible

	commands.clear();
}

void Unit::Destroy()
{
	timer.Start();
	ex_state = DESTROYED;
	App->entitycontroller->selected_entities.remove(this);
	App->entitycontroller->operative_entities.remove(this);
	isSelected = false;
	if (App->tutorial->doingTutorial && IsEnemy())
	{
		std::vector<Unit*> units;
		squad->getUnits(units);
		if (units.empty()) //It was last enemy
			App->tutorial->taskCompleted(KILL_ENEMIES);
	}
	App->gui->entityDeleted(this);
	Halt();
}

void Unit::calculateSeparationVector(fPoint& separation_v, float& weight)
{
	std::vector<Entity*> collisions;
	App->entitycontroller->colliderQT->FillCollisionVector(collisions, collider);

	separation_v = { 0.0f,0.0f };
	weight = 0;

	for (int i = 0; i < collisions.size(); i++)
	{
		if(collisions[i]->ex_state != DESTROYED && collisions[i]->isActive && collisions[i]->IsEnemy() == IsEnemy() && collisions[i] != this)
		{
			fPoint current_separation = (position - collisions[i]->position);

			if (current_separation.GetModule() < collider.w)
			{
				while (current_separation.IsZero())
				{
					current_separation = { (float)(rand() % 10), (float)(rand() % 10) };
					current_separation.Normalized();
				}
				separation_v += (current_separation.Normalized() * (current_separation.GetModule() - collider.w)).Negate();
			}
		}
	}

	if (!separation_v.IsZero())
	{
		weight = separation_v.GetModule() / (collider.w * 2);

		if (weight > MAX_SEPARATION_WEIGHT)
			weight = MAX_SEPARATION_WEIGHT;

		separation_v.Normalize();
		fPoint world_separation = position + (separation_v * weight);
		iPoint map_separation = App->map->WorldToMap(world_separation.x, world_separation.y);

		if (!App->pathfinding->IsWalkable(map_separation))
			weight = 0;
	}
}



void Unit::animationController()
{
	if(getCurrentCommand() != ATTACK)
		lookAt(mov_direction);

	animationType new_animation = IDLE_S;
	if (current_HP > 0)
	{
		if (mov_module > 0.5)
		{
			switch (commands.empty() ? MOVETO : commands.front()->type)
			{
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
			default:
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
			if (idle_timer.ReadMs() > 15000)
			{
				idle_timer.Start();
				int number = rand() % 7;
				dir = (direction)number;
			}
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
	}
	else 
	{
		switch (dir)
		{
		case E:  new_animation = DEAD_SE; break;
		case SE: new_animation = DEAD_SE; break;
		case NE: new_animation = DEAD_NE; break;
		case N:  new_animation = DEAD_NE; break;
		case S:  new_animation = DEAD_SW; break;
		case W:  new_animation = DEAD_NW; break;
		case NW: new_animation = DEAD_NW; break;
		case SW: new_animation = DEAD_SW; break;
		}
	}

	if (last_anim == new_animation)
	{
		if (animations[new_animation] != current_anim)
		{
			current_anim = animations[new_animation];
			current_anim->Reset();
		}
	}
	
	last_anim = new_animation;
}


//buffs
void Unit::AddDamagebuff(int duration, int amount, operation_sign sign)
{
	Effect*  ret = new DamageBuff(amount, duration, this, sign);
	switch (sign)
	{
	case PLUS_MINUS:
	{
		effects.push_back(ret);
		break;
	}
	case MULTIPLICATION_DIVISION:
	{
		effects.push_front(ret);
		break;
	}
	default:
	{
		effects.push_back(ret);
		break;
	}
	}

}
void Unit::AddPiercingDamagebuff(int duration, int amount, operation_sign sign)
{
	Effect*  ret = new PiercingDamageBuff(amount, duration, this, sign);
	switch (sign)
	{
	case PLUS_MINUS:
	{
		effects.push_back(ret);
		break;
	}
	case MULTIPLICATION_DIVISION:
	{
		effects.push_front(ret);
		break;
	}
	default:
	{
		effects.push_back(ret);
		break;
	}
	}

}
void Unit::AddSpeedbuff(int duration, int amount, operation_sign sign)
{
	Effect*  ret = new SpeedBuff(amount, duration, this, sign);
	switch (sign)
	{
	case PLUS_MINUS:
	{
		effects.push_back(ret);
		break;
	}
	case MULTIPLICATION_DIVISION:
	{
		effects.push_front(ret);
		break;
	}
	default:
	{
		effects.push_back(ret);
		break;
	}
	}

}
void Unit::AddDefensebuff(int duration, int amount, operation_sign sign)
{
	Effect*  ret = new DefenseBuff(amount, duration, this, sign);
	switch (sign)
	{
	case PLUS_MINUS:
	{
		effects.push_back(ret);
		break;
	}
	case MULTIPLICATION_DIVISION:
	{
		effects.push_front(ret);
		break;
	}
	default:
	{
		effects.push_back(ret);
		break;
	}
	}

}
void Unit::AddRangebuff(int duration, int amount, operation_sign sign)
{
	Effect*  ret = new DamageBuff(amount, duration, this, sign);
	switch (sign)
	{
	case PLUS_MINUS:
	{
		effects.push_back(ret);
		break;
	}
	case MULTIPLICATION_DIVISION:
	{
		effects.push_front(ret);
		break;
	}
	default:
	{
		effects.push_back(ret);
		break;
	}
	}

}