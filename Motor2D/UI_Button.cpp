#include "UI_Button.h"
#include "j1App.h"
#include "j1Render.h"
#include "Brofiler\Brofiler.h"

void Button::BlitElement(bool use_camera)
{
	if (active)
	{
		BROFILER_CATEGORY("Button Blit", Profiler::Color::DarkKhaki);
		iPoint globalPos = calculateAbsolutePosition();
		switch (state)
		{
		case STANDBY:
			App->render->Blit(texture, globalPos.x, globalPos.y, &section, use_camera);
			break;
		case MOUSEOVER:
			App->render->Blit(texture, globalPos.x, globalPos.y, &OnMouse, use_camera);
			break;
		case LOCKED:
		case LOCKED_MOUSEOVER:
		case CLICKED:
			App->render->Blit(texture, globalPos.x, globalPos.y, &OnClick, use_camera);
			break;
		}

		if (displayingHotkey && hotkey_text != nullptr)
		{
			hotkey_text->BlitElement(use_camera);
		}

		UI_element::BlitElement(use_camera);
	}
}

void Button::displayHotkey(bool display, _TTF_Font* font)
{
	displayingHotkey = display;

	if (display = true && font != nullptr && Hotkey != 0)
	{
		if (hotkey_text != nullptr)
			RELEASE(hotkey_text);
		
		std::string text;
		text = (char)(Hotkey + 61);

		hotkey_text = new Text(text, 0, 0, font, { 255,0,0,255 }, nullptr);
		iPoint position = { localPosition.x + section.w - hotkey_text->section.w / 2, localPosition.y + section.h - hotkey_text->section.h / 2 };
		hotkey_text->localPosition = position;
	}
}

void Button::setHotkey(SDL_Scancode hotkey)
{
	Hotkey = hotkey;
}

SDL_Scancode Button::getHotkey() const
{
	return Hotkey;
}

void Button::updatedPosition()
{
	if (hotkey_text != nullptr)
	{
		iPoint position = { localPosition.x + section.w - hotkey_text->section.w / 2, localPosition.y + section.h - hotkey_text->section.h / 2 };
		hotkey_text->localPosition = position;
	}
}
