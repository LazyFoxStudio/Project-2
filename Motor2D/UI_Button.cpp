#include "UI_Button.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Fonts.h"
#include "Brofiler\Brofiler.h"

void Button::BlitElement()
{
	if (active)
	{
		SDL_RendererFlip flip = SDL_FLIP_NONE;
		if (flipHorizontal)
			flip = SDL_FLIP_HORIZONTAL;
		else if (flipVertical)
			flip = SDL_FLIP_VERTICAL;

		BROFILER_CATEGORY("Button Blit", Profiler::Color::DarkKhaki);
		iPoint globalPos = calculateAbsolutePosition();
		switch (state)
		{
		case STANDBY:
			if (this->element_type == BUTTON || !switchOn)
				App->render->Blit(texture, globalPos.x, globalPos.y, &section, use_camera, true, 1.0f, flip);
			else
				App->render->Blit(texture, globalPos.x, globalPos.y, &OnClick, use_camera, true, 1.0f, flip);
			break;
		case MOUSEOVER:
			if (this->element_type == BUTTON || !switchOn)
				App->render->Blit(texture, globalPos.x, globalPos.y, &OnMouse, use_camera, true, 1.0f, flip);
			else
				App->render->Blit(texture, globalPos.x, globalPos.y, &OnMouseOn, use_camera, true, 1.0f, flip);
			break;
		case LOCKED:
		case LOCKED_MOUSEOVER:
		case CLICKED:
			App->render->Blit(texture, globalPos.x, globalPos.y, &OnClick, use_camera, true, 1.0f, flip);
			break;
		}

		if (hasCooldown && inCooldown)
		{
			if (cooldown_timer.Read() >= cooldown * 1000)
			{
				inCooldown = false;
				Unlock();
			}
			else
			{
				float percentage = (cooldown_timer.Read()/1000.0f) / (float)cooldown;
				App->render->DrawQuad({ globalPos.x, globalPos.y + (int)(section.h*percentage), section.w, section.h - (int)(section.h*percentage) }, Translucid_White, true, false, true);
			}
		}

		UI_element::BlitElement();
	}
}

void Button::displayHotkey(bool display, _TTF_Font* font, SDL_Color color, bool atCenter)
{
	displayingHotkey = display;

	if (display == true && font != nullptr && Hotkey != 0)
	{
		if (hotkey_text != nullptr)
		{
			childs.remove(hotkey_text);
			RELEASE(hotkey_text);
		}
		
		std::string text;
		text = (char)(Hotkey + 61);

		hotkey_text = new Text(text, 0, 0, font, color, nullptr);
		hotkey_text->interactive = false;
		appendChild(hotkey_text, true);
		if (!atCenter)
			hotkey_text->localPosition = { section.w - hotkey_text->section.w / 2, section.h - hotkey_text->section.h / 2 };
	}
}

void Button::setHotkey(SDL_Scancode hotkey)
{
	Hotkey = hotkey;
	if (hotkey_text != nullptr)
	{
		std::string text;
		text = (char)(Hotkey + 61);
		hotkey_text->setText(text);
	}
	cancelReadingHotkey();
}

SDL_Scancode Button::getHotkey() const
{
	return Hotkey;
}

void Button::setReadingHotkey()
{
	if (readingHotkeyLine1 == nullptr || readingHotkeyLine2 == nullptr)
	{
		readingHotkeyLine1 = new Text("Press a", 15, 25, App->font->getFont(2), { 75, 0, 0, 255 }, nullptr);
		readingHotkeyLine1->interactive = false;
		appendChild(readingHotkeyLine1);
		readingHotkeyLine2 = new Text("letter key", 5, 45, App->font->getFont(2), { 75, 0, 0, 255 }, nullptr);
		readingHotkeyLine2->interactive = false;
		appendChild(readingHotkeyLine2);
	}
	else
	{
		readingHotkeyLine1->active = true;
		readingHotkeyLine2->active = true;
	}
	hotkey_text->active = false;
}

void Button::cancelReadingHotkey()
{
	if (readingHotkeyLine1 != nullptr)
		readingHotkeyLine1->active = false;
	if (readingHotkeyLine2 != nullptr)
		readingHotkeyLine2->active = false;
	if (hotkey_text != nullptr)
		hotkey_text->active = true;
}

void Button::setCooldown(int cooldown)
{
	hasCooldown = true;
	this->cooldown = cooldown;
}

void Button::Used()
{
	if (hasCooldown && !inCooldown)
	{
		inCooldown = true;
		cooldown_timer.Start();
		Lock();
		setCondition("In cooldown");
	}
}
