#include "UI_CooldownsDisplay.h"
#include "j1App.h"
#include "UI_Text.h"
#include "UI_Chrono.h"
#include "j1Fonts.h"
#include "j1EntityController.h"
#include "Hero.h"
#include "Skills.h"

CooldownsDisplay::CooldownsDisplay(): UI_element(0,0,TEXT, {0,0,0,0}, nullptr)
{
	hero = new Text("Hero will respawn in: ", 5, 60, App->font->getFont(1), { 255,255,0,255 }, nullptr);
	hero->active = false;
	hero_timer = new Chrono(210, 60, TIMER, App->font->getFont(1), { 255,255,0,255 }, nullptr);
	hero_timer->setStartValue(HERO_REVIVE_COOLDOWN);
	hero_timer->active = false;
	Hero* hero = (Hero*)App->entitycontroller->getEntitybyID(App->entitycontroller->hero_UID);
	skill1 = new Chrono(0, 0, TIMER, App->font->getFont(1), { 255,255,255,255 }, nullptr);
	skill1->setStartValue(hero->skill_one->cooldown);
	skill2 = new Chrono(0, 0, TIMER, App->font->getFont(1), { 255,255,255,255 }, nullptr);
	skill2->setStartValue(hero->skill_two->cooldown);
	skill3 = new Chrono(0, 0, TIMER, App->font->getFont(1), { 255,255,255,255 }, nullptr);
	skill3->setStartValue(hero->skill_three->cooldown);
}


CooldownsDisplay::~CooldownsDisplay()
{
	RELEASE(hero);
	RELEASE(hero_timer);
}

void CooldownsDisplay::BlitElement()
{
	hero->BlitElement();
	hero_timer->BlitElement();

	Hero* heroE = (Hero*)App->entitycontroller->getEntitybyID(App->entitycontroller->hero_UID);

	if (heroE->current_skill == 1 && !heroE->skill_one->Ready())
	{
		App->gui->moveElementToMouse(skill1);
		skill1->BlitElement();
	}
	else if (heroE->current_skill == 2 && !heroE->skill_two->Ready())
	{
		App->gui->moveElementToMouse(skill2);
		skill2->BlitElement();
	}
	else if (heroE->current_skill == 3 && !heroE->skill_three->Ready())
	{
		App->gui->moveElementToMouse(skill3);
		skill3->BlitElement();
	}

	if (hero_timer->time == 0)
	{
		hero_timer->active = false;
		hero->active = false;
	}
	if (skill1->time == 0)
		skill1->active = false;
	if (skill2->time == 0)
		skill2->active = false;
	if (skill3->time == 0)
		skill3->active = false;
}

void CooldownsDisplay::heroDead()
{
	hero_timer->active = true;
	hero->active = true;
	hero_timer->restartChrono();
}

void CooldownsDisplay::skillUsed(int skill)
{
	switch (skill)
	{
	case 1:
		skill1->active = true;
		skill1->restartChrono();
		break;
	case 2:
		skill2->active = true;
		skill2->restartChrono();
		break;
	case 3: 
		skill3->active = true;
		skill3->restartChrono();
		break;
	}
}

void CooldownsDisplay::Reset()
{
	hero_timer->active = false;
	hero->active = false;
	skill1->active = false;
	skill2->active = false;
	skill3->active = false;
}
