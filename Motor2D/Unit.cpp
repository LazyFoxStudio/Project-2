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

#define SPEED_CONSTANT 1.2f   // applied to all units       
#define STOP_TRESHOLD 5.0f			

#define MAX_MOVEMENT_WEIGHT 100.0f   // max value for the next_step vector, for steering calculations  /
#define MAX_SEPARATION_WEIGHT 200.0f   
#define MAX_COHESION_WEIGHT 80.0f
#define MAX_ALIGNEMENT_WEIGHT 50.0f

#define SEPARATION_STRENGTH 8.0f
#define COHESION_STRENGTH 1.0f    // the lower the stronger
#define ALIGNEMENT_STRENGTH 5.0    // the lower the stronger

#define STEERING_FACTOR 1.5f

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
	if ((!commands.empty() ? commands.front()->type != ATTACK : true))
	{
		fPoint separation_v, cohesion_v, alignement_v;
		int separation_w, cohesion_w, alignement_w;

		calculateSeparationVector(separation_v, separation_w);
		calculateCohesionVector(cohesion_v, cohesion_w);
		calculateAlignementVector(alignement_v, alignement_w);

		fPoint displacement = ((movement * MAX_MOVEMENT_WEIGHT) + (separation_v * separation_w) + (cohesion_v * cohesion_w) + (alignement_v * alignement_w));
		
		if (displacement.GetModule() < STOP_TRESHOLD)
		{
			if (displacement.GetModule() < 1)
				next_step.SetToZero();
			else
				next_step = { next_step.x * 0.9f, next_step.y * 0.9f };
		}
		else
			next_step = (next_step * STEERING_FACTOR) + displacement;

		if (next_step.GetModule() > MAX_MOVEMENT_WEIGHT)
			next_step = next_step.Normalized() * MAX_MOVEMENT_WEIGHT;

		displacement = (next_step * (squad ? squad->max_speed : speed) * dt * SPEED_CONSTANT);

		if (displacement.GetModule() > 5)
			displacement = displacement.Normalized() * 5;

		if (App->pathfinding->IsWalkable(App->map->WorldToMap(position.x + displacement.x, position.y + displacement.y)))
		{
			position += displacement;

			collider.x = position.x - (collider.w / 2);
			collider.y = position.y - (collider.h / 2);
		}
	}
	else if (!next_step.IsZero()) next_step.SetToZero();
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
	App->gui->entityDeleted(this);
	ex_state = DESTROYED;
	App->entitycontroller->selected_entities.remove(this);
}

void Unit::calculateSeparationVector(fPoint& separation_v, int& weight)
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
			if (current_separation.GetModule() < (collider.w / 2.0f))
			{
				current_separation = current_separation.Normalized() * (collider.w * SEPARATION_STRENGTH / current_separation.GetModule());
				separation_v += current_separation;
			}
		}
	}

	if (!separation_v.IsZero())
	{
		weight = separation_v.GetModule();

		if (weight > MAX_SEPARATION_WEIGHT)
			weight = MAX_SEPARATION_WEIGHT;

		separation_v.Normalize();
		fPoint world_separation = position + (separation_v * weight);
		iPoint map_separation = App->map->WorldToMap(world_separation.x, world_separation.y);

		if (!App->pathfinding->IsWalkable(map_separation))
			weight = 0;
	}
}

void Unit::calculateCohesionVector(fPoint& cohesion_v, int& weight)
{
	cohesion_v = { 0.0f,0.0f };
	weight = 0;

	if (getCurrentCommand() != ATTACKING_MOVETO)
	{
		if (squad ? (!squad->centroid.IsZero() && squad->units_id.size() > 1) : false)
		{
			cohesion_v = ((squad->centroid + squad->getOffset(UID)) - position);
			if (cohesion_v.GetModule() > collider.w / 4)
			{
				weight = cohesion_v.GetModule() / COHESION_STRENGTH;
				cohesion_v.Normalize();

				if (weight > MAX_COHESION_WEIGHT)
					weight = MAX_COHESION_WEIGHT;

				fPoint world_offset = (position + (cohesion_v * weight));
				iPoint map_offset = App->map->WorldToMap(world_offset.x, world_offset.y);

				if (!App->pathfinding->IsWalkable(map_offset))
					weight = 0;
			}
			else
				cohesion_v.SetToZero();
		}
	}
}

void Unit::calculateAlignementVector(fPoint& alignement_v, int& weight)
{
	alignement_v = { 0.0f, 0.0f };
	weight = 0;

	if (getCurrentCommand() != ATTACKING_MOVETO)
	{
		if (squad ? !squad->squad_movement.GetModule() > 1.0f : false)
		{
			alignement_v = squad->squad_movement.Normalized();
			weight = (squad->squad_movement.GetModule() / ALIGNEMENT_STRENGTH) * MAX_ALIGNEMENT_WEIGHT;

			if (weight > MAX_ALIGNEMENT_WEIGHT)
				weight = MAX_ALIGNEMENT_WEIGHT;

			fPoint world_alignement = (position + (alignement_v * weight));
			iPoint map_alignement = App->map->WorldToMap(world_alignement.x, world_alignement.y);

			if (!App->pathfinding->IsWalkable(map_alignement))
				weight = 0;
		}
	}

}


void Unit::animationController()
{
	lookAt(next_step);

	animationType new_animation = IDLE_S;
	if (ex_state != DESTROYED)
	{
		if (next_step.GetModule() > STOP_TRESHOLD)
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