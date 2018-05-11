#include "Hero.h"
#include "j1Render.h"
#include "j1Input.h"
#include "Color.h"
#include "Command.h"
#include "j1EntityController.h"
#include "j1ParticleController.h"
#include "Squad.h"
#include "j1Gui.h"
#include "j1Audio.h"
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
	if (current_HP <= 0)
	{
		if (isActive)
		{
			Deactivate();
			App->gui->cooldownsDisplay->heroDead();
		}
		else if (timer.ReadSec() > HERO_REVIVE_COOLDOWN)
		{
			current_HP = max_HP; 
			setActive(true);
			App->gui->createLifeBar(this);
		}
		return true;
	}

	animationController();

	switch (current_skill)
	{
	case 1: 
		skill_one->DrawRange(); 
		if ((App->input->GetMouseButtonDown(1) == KEY_DOWN && skill_one->Ready()) || skill_one->going)
		{
			skill_one->Activate();
			if (!skill_one->going)
			{
				iPoint tmp;
				App->input->GetMousePosition(tmp.x,tmp.y);
				fPoint tmp2(tmp.x, tmp.y);
				
				if( type == HERO_1)
					App->audio->PlayFx(SFX_HERO_YHAMAM_ICICLECRASH);
				else if(type == HERO_2)
					App->audio->PlayFx(SFX_CONSECRATION);

				App->particle->AddParticle(PICICLE_CRASH, tmp2);
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
				iPoint tmp;
				App->input->GetMousePosition(tmp.x, tmp.y);
				//tmp = App->map->WorldToMap(tmp.x, tmp.y);
				//tmp = App->map->MapToWorld(tmp.x, tmp.y);
				fPoint tmp2(tmp.x, tmp.y);

				App->audio->PlayFx(SFX_HERO_YHAMAM_OVERFLOW);
				
				
				if (type == HERO_1)
				{
					App->audio->PlayFx(SFX_HERO_YHAMAM_OVERFLOW);
					App->particle->AddParticle(POVERFLOW, { tmp2.x, tmp2.y - 30 });
				}
				else if (type == HERO_2)
					App->audio->PlayFx(SFX_CIRCLE_OF_LIGHT);
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
				if(type == HERO_1)
					App->audio->PlayFx(SFX_HERO_YHAMAM_DRAGONBREATH);
				else if(type==HERO_2)
					App->audio->PlayFx(SFX_HONOR_OF_THE_PURE);
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
