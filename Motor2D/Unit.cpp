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
#include "Minimap.h"

#define SEPARATION_WEIGHT 1.5f   // the higher the stronger   // 1.0f ~ 10.0f//
#define SPEED_CONSTANT 5.0f   // applied to all units            // 60 ~ 140 //
#define STOP_TRESHOLD 0.5f			// 0.5f ~ 1.5f//

#define COHESION_WEIGHT 1.5f
#define COHESION_TRESHOLD 10.0f

#define MOVEMENT_WEIGHT 5.0f
#define ALIGNEMENT_WEIGHT 8.0f

#define MINIMUM_TRESHOLD 0.2f

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
	if (!isActive) return;

	SDL_Rect r = current_anim->GetCurrentFrame(dt);

	if(dir == W  || dir == NW || dir == SW)
		App->render->Blit(texture, position.x - (r.w / 2), position.y - (r.h / 2), &r, true,false,(1.0F), SDL_FLIP_HORIZONTAL);
	else
		App->render->Blit(texture, position.x - (r.w / 2), position.y - (r.h / 2), &r);
	
}

bool Unit::Update(float dt)
{
	animationController();

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
		float time_passed= (*it)->timer.ReadSec();
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


	if (current_HP <= 0)
	{
		if (ex_state != DESTROYED) { Destroy(); squad->removeUnit(UID); }

		else if (timer.ReadSec() > DEATH_TIME)  return false;
		return true;
	}

	if (!commands.empty())
	{
		commands.front()->Execute(dt);
		if (commands.front()->state == FINISHED) commands.pop_front();
	}

	//minimap_
	if (App->gui->minimap)
	{
		if (IsEnemy()) App->gui->minimap->Addpoint({ (int)position.x,(int)position.y,50,50 }, Red);
		else		   App->gui->minimap->Addpoint({ (int)position.x,(int)position.y,50,50 }, Green);
	}

	Move(dt);	

	return true;
}

void Unit::Move(float dt)
{
	fPoint separation_v = calculateSeparationVector();
	fPoint cohesion_v = calculateCohesionVector();
	fPoint alignement_v = calculateAlignementVector();

	if ((!commands.empty() ? commands.front()->type != ATTACK : false) || separation_v.GetModule() > STOP_TRESHOLD || cohesion_v.GetModule()> STOP_TRESHOLD || alignement_v.GetModule() > STOP_TRESHOLD)
	{
		next_step = next_step + ((movement * MOVEMENT_WEIGHT) + separation_v + cohesion_v + alignement_v) * STEERING_FACTOR;

		if (next_step.GetModule() > MAX_NEXT_STEP_MODULE)
			next_step = next_step.Normalized() * MAX_NEXT_STEP_MODULE;

		fPoint last_pos = position;

		if (squad)	position += (next_step * squad->max_speed * dt * SPEED_CONSTANT);
		else		position += (next_step * speed * dt * SPEED_CONSTANT);

		if (!App->pathfinding->IsWalkable(App->map->WorldToMap(position.x, position.y))) 
		{ 
			position = last_pos;
			next_step.SetToZero();
		}

		collider.x = position.x - (collider.w / 2);
		collider.y = position.y - (collider.h / 2);
	}
	else if (!next_step.IsZero()) next_step.SetToZero();
}

void Unit::lookAt(fPoint direction)
{
	if (direction.x != 0 || direction.y != 0)
	{
		if (direction.x < -MAX_NEXT_STEP_MODULE / 2)
		{
			if (direction.y < MAX_NEXT_STEP_MODULE / 4 && direction.y > -MAX_NEXT_STEP_MODULE / 4) dir = W;
			else dir = (direction.y < 0 ? NW : SW);
		}
		else if (direction.x > MAX_NEXT_STEP_MODULE / 2)
		{
			if (direction.y < MAX_NEXT_STEP_MODULE / 4 && direction.y > -MAX_NEXT_STEP_MODULE / 4) dir = E;
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
	App->gui->entityDeleted(this);
	ex_state = DESTROYED;
	App->entitycontroller->selected_entities.remove(this);
}

fPoint Unit::calculateSeparationVector()
{
	SDL_Rect r = { collider.x - COLLIDER_MARGIN, collider.y - COLLIDER_MARGIN , collider.w + COLLIDER_MARGIN , collider.h + COLLIDER_MARGIN };
	std::vector<Entity*> collisions;
	App->entitycontroller->CheckCollidingWith(r, collisions, this);

	fPoint separation_v = { 0.0f,0.0f };

	for (int i = 0; i < collisions.size(); i++)
	{
		if ((collisions[i])->IsEnemy() == IsEnemy())
		{
			separation_v += (position - collisions[i]->position);
		}
	}
	separation_v = separation_v.Normalized() * SEPARATION_WEIGHT;

	fPoint world_separation = position + separation_v;
	iPoint map_separation = App->map->WorldToMap(world_separation.x, world_separation.y);

	if (!App->pathfinding->IsWalkable(map_separation) || separation_v.GetModule() > MINIMUM_TRESHOLD)
		return { 0.0f, 0.0f };

	return separation_v;
}

fPoint Unit::calculateCohesionVector()
{
	fPoint cohesion_v = { 0.0f,0.0f };
	if (squad ? (!squad->centroid.IsZero() && squad->units_id.size() > 1) : false)
	{
		cohesion_v = (squad->centroid + squad->getOffset(UID)) - position;
		cohesion_v = cohesion_v.Normalized() * (cohesion_v.GetModule() / COHESION_TRESHOLD);
		cohesion_v = { cohesion_v.x * COHESION_WEIGHT, cohesion_v.y * COHESION_WEIGHT };

		fPoint world_offset = (squad->centroid + squad->getOffset(UID));
		iPoint map_offset = App->map->WorldToMap(world_offset.x, world_offset.y);

		if (!App->pathfinding->IsWalkable(map_offset) || cohesion_v.GetModule() < MINIMUM_TRESHOLD)
			return { 0.0f, 0.0f };

	}
	return cohesion_v;
}

fPoint Unit::calculateAlignementVector()
{
	fPoint alignement_v = { 0.0f, 0.0f };

	if (squad ? !squad->squad_movement.IsZero() : false)
	{
		alignement_v = ((squad->squad_movement * (squad->max_speed / 5)) * ALIGNEMENT_WEIGHT);

		fPoint world_alignement = (squad->centroid + squad->getOffset(UID));
		iPoint map_alignement = App->map->WorldToMap(world_alignement.x, world_alignement.y);

		if (!App->pathfinding->IsWalkable(map_alignement) || alignement_v.GetModule() < MINIMUM_TRESHOLD)
			return { 0.0f, 0.0f };
	}

	return alignement_v;
}


void Unit::animationController()
{
	lookAt(next_step);

	animationType new_animation = IDLE_S;
	if (ex_state != DESTROYED)
	{
		if (!next_step.IsZero())
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

	if (animations[new_animation] != current_anim)
	{
		current_anim = animations[new_animation];
		current_anim->Reset();
	}

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