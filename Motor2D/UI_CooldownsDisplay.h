#ifndef __UI_COOLDOWNSDISPLAY_H__
#define __UI_COOLDOWNSDISPLAY_H__

#include "UI_element.h"

class Text;
class Chrono;
class Hero;

class CooldownsDisplay : public UI_element
{
public:
	CooldownsDisplay();
	~CooldownsDisplay();

	void BlitElement();
	void heroDead();
	void skillUsed(int skill);

	void Reset();
	void heroChoosen(Hero* hero);

public:

	Text* hero = nullptr;
	Chrono* hero_timer = nullptr;
	Chrono* skill1 = nullptr;
	Chrono* skill2 = nullptr;
	Chrono* skill3 = nullptr;
};

#endif //__UI_COOLDOWNSDISPLAY_H__