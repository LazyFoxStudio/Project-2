#include "UI_UnlockDisplay.h"
#include "j1App.h"
#include "UI_Image.h"
#include "UI_Text.h"
#include "j1Fonts.h"

UnlockDisplay::UnlockDisplay()
{
}


UnlockDisplay::~UnlockDisplay()
{
	cleanUnlockments();
}

void UnlockDisplay::BlitElement()
{
	for (std::list<Unlockment*>::iterator it_u = unlockments.begin(); it_u != unlockments.end(); it_u++)
	{
		if ((*it_u)->title != nullptr)
			(*it_u)->title->BlitElement();
		if ((*it_u)->icon != nullptr)
			(*it_u)->icon->BlitElement();
		if ((*it_u)->name != nullptr)
			(*it_u)->name->BlitElement();

		if ((*it_u)->timer.Read() >= DEFAULT_UNLOCK_DISPLAY_TIME)
		{
			unlockments.erase(it_u);
			RELEASE((*it_u));
		}
	}
}

void UnlockDisplay::unlockedAbility(SDL_Rect icon, std::string name)
{
	Unlockment* newUnlock = new Unlockment();

	int Y_offset = 0 + (80 * unlockments.size());

	newUnlock->title = new Text("Ability Unlocked:", 20, 675 - Y_offset, App->font->getFont(8), { 255,255,255,255 }, nullptr);
	newUnlock->title->setOutlined(true);
	newUnlock->icon = new Image(App->gui->atlas, 275, 663 - Y_offset, icon, nullptr);
	newUnlock->name = new Text(name, 360, 675 - Y_offset, App->font->getFont(8), { 255,255,255,255 }, nullptr);
	newUnlock->name->setOutlined(true);
	newUnlock->timer.Start();

	unlockments.push_back(newUnlock);
}

void UnlockDisplay::unlockedBuilding(SDL_Rect icon, std::string name)
{
	Unlockment* newUnlock = new Unlockment();

	int Y_offset = 0 + (80 * unlockments.size());

	newUnlock->title = new Text("Building Unlocked:", 20, 675 - Y_offset, App->font->getFont(8), { 255,255,255,255 }, nullptr);
	newUnlock->title->setOutlined(true);
	newUnlock->icon = new Image(App->gui->atlas, 275, 663 - Y_offset, icon, nullptr);
	newUnlock->name = new Text(name, 360, 675 - Y_offset, App->font->getFont(8), { 255,255,255,255 }, nullptr);
	newUnlock->name->setOutlined(true);
	newUnlock->timer.Start();

	unlockments.push_back(newUnlock);
}

void UnlockDisplay::cleanUnlockments()
{
	std::list<Unlockment*>::iterator it_u;
	it_u = unlockments.begin();
	while (it_u != unlockments.end())
	{
		RELEASE((*it_u));
		it_u++;
	}
	unlockments.clear();
}

Unlockment::~Unlockment()
{
	RELEASE(title);
	RELEASE(icon);
	RELEASE(name);
}
