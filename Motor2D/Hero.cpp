#include "Hero.h"
#include "j1Render.h"
#include "j1Input.h"
#include "Color.h"
#include "Command.h"
#include "j1EntityController.h"
#include "j1ParticleController.h"
#include "j1Pathfinding.h"
#include "Squad.h"
#include "j1Gui.h"
#include "j1Audio.h"
#include "j1Map.h"
#include "UI_CooldownsDisplay.h"
#include "Minimap.h"
#include "j1Tutorial.h"

Hero::~Hero()
{
	Halt();
	animations.clear();
	effects.clear();

	RELEASE(skill_one);
	RELEASE(skill_two);
	RELEASE(skill_three);
}

bool Hero::Update(float dt)
{
	animationController();

	if (current_HP <= 0)
	{
		if (isActive)
		{
			Deactivate();
			App->gui->cooldownsDisplay->heroDead();
		}
		else if (timer.ReadSec() > HERO_REVIVE_COOLDOWN)
		{
			iPoint th_w_pos = TOWN_HALL_POS;
			iPoint th_m_pos = App->map->WorldToMap(th_w_pos.x, th_w_pos.y);
			th_m_pos = App->pathfinding->FirstWalkableAdjacentSafeProof(th_m_pos, App->map->WorldToMap(position.x, position.y));

			if (th_m_pos != iPoint(-1, -1))
			{
				th_w_pos = App->map->MapToWorld(th_m_pos.x, th_m_pos.y);
				position = { (float)th_w_pos.x + App->map->data.tile_width / 2, (float)th_w_pos.y + App->map->data.tile_height / 2 };
			}
			current_HP = max_HP; 
			setActive(true);
			ex_state = OPERATIVE;
			App->gui->createLifeBar(this);
		}
		return true;
	}

	switch (current_skill)
	{
	case 1: 
		skill_one->DrawRange(); 
		if ((App->input->GetMouseButtonDown(1) == KEY_DOWN && skill_one->Ready()) || skill_one->going)
		{
			skill_one->Activate();
			if (!skill_one->going)
			{			
				if (type == HERO_1)
				{
					App->audio->PlayFx(SFX_HERO_YHAMAM_ICICLECRASH);

					if (skill_one->last_cast.IsZero())
						App->particle->AddParticle(PICICLE_CRASH, { (float)skill_one->cast_pos.x, (float)(skill_one->cast_pos.y) }, false);
					else
						App->particle->AddParticle(PICICLE_CRASH, { (float)skill_one->last_cast.x, (float)(skill_one->last_cast.y) }, false);
				}
				else if (type == HERO_2)
				{
					App->audio->PlayFx(SFX_CONSECRATION);
					App->particle->AddParticle(PHOLYDAMAGE, position, false);

				}
				skill_one->last_cast.SetToZero();
			}
			App->gui->cooldownsDisplay->skillUsed(1);
		}
		skill_one->toDraw.clear();
		break;
	case 2:
		skill_two->DrawRange();
		if (App->input->GetMouseButtonDown(1) == KEY_DOWN && skill_two->Ready() || skill_two->going)
		{
			skill_two->Activate();
			if (!skill_two->going)
			{		
				if (type == HERO_1)
				{
					App->audio->PlayFx(SFX_HERO_YHAMAM_OVERFLOW);
					if (skill_two->last_cast.IsZero())
						App->particle->AddParticle(POVERFLOW, { (float)skill_two->cast_pos.x, (float)(skill_two->cast_pos.y - 30) }, false);
					else
						App->particle->AddParticle(POVERFLOW, { (float)skill_two->last_cast.x, (float)(skill_two->last_cast.y - 30) }, false);
				}
				else if (type == HERO_2)
				{
					App->audio->PlayFx(SFX_CIRCLE_OF_LIGHT);
					if (skill_two->last_cast.IsZero())
						App->particle->AddParticle(PHOLYHEAL, { (float)skill_two->cast_pos.x, (float)(skill_two->cast_pos.y) }, false);
					else
						App->particle->AddParticle(PHOLYHEAL, { (float)skill_two->last_cast.x, (float)(skill_two->last_cast.y) }, false);
				}
				skill_two->last_cast.SetToZero();
			}
			App->gui->cooldownsDisplay->skillUsed(2);
		}
		skill_two->toDraw.clear();		
		break;
	case 3:
		skill_three->DrawRange();
		if (App->input->GetMouseButtonDown(1) == KEY_DOWN && skill_three->Ready() || skill_three->going)
		{
			skill_three->Activate();

			if (!skill_three->going)
			{
				if (type == HERO_1)
				{
					if (skill_three->last_cast.IsZero())
						App->particle->AddProgressiveParticle(PFLAMETHROWER, position, { (float)skill_three->cast_pos.x, (float)(skill_three->cast_pos.y) }, 2.5f, 10, false);
					else
						App->particle->AddProgressiveParticle(PFLAMETHROWER, position, { (float)skill_three->last_cast.x, (float)(skill_three->last_cast.y) }, 2.5f, 10, false);
					App->audio->PlayFx(SFX_HERO_YHAMAM_DRAGONBREATH);
				}
				else if(type==HERO_2)
				{
					App->audio->PlayFx(SFX_HONOR_OF_THE_PURE);
					App->particle->AddParticle(PHOLYBUFF, position, false);
				}
				skill_three->last_cast.SetToZero();

			}
			App->gui->cooldownsDisplay->skillUsed(3);
		}
		skill_three->toDraw.clear();	
		break;
	default: 
		break;
	}

	if (App->input->GetMouseButtonDown(3) == KEY_DOWN && current_skill != 0)
	{
		skill_one->going = false;
		skill_two->going = false;
		skill_three->going = false;
		current_skill = 0;
	}


	if (!commands.empty())
	{
		commands.front()->Execute(dt);
		if (commands.front()->state == FINISHED) commands.pop_front();
	}

	Move(dt);

	//minimap_
	if (App->gui->minimap)
		App->gui->minimap->Addpoint({ (int)position.x,(int)position.y,75,75 }, White);

	return true;
}

void Hero::Deactivate()
{
	Halt();
	mov_direction.SetToZero();
	mov_module = 0.0f;
	setActive(false);
	timer.Start();
	App->entitycontroller->selected_squads.remove(squad);
	App->entitycontroller->selected_entities.remove(this);
	App->gui->entityDeleted(this);
	current_skill = 0;
}
