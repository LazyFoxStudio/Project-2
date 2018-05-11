#include "UI_Button.h"
#include "j1App.h"
#include "j1Render.h"
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
			App->render->Blit(texture, globalPos.x, globalPos.y, &section, use_camera, true, 1.0f, flip);
			break;
		case MOUSEOVER:
			App->render->Blit(texture, globalPos.x, globalPos.y, &OnMouse, use_camera, true, 1.0f, flip);
			break;
		case LOCKED:
		case LOCKED_MOUSEOVER:
		case CLICKED:
			App->render->Blit(texture, globalPos.x, globalPos.y, &OnClick, use_camera, true, 1.0f, flip);
			break;
		}

		if (displayingHotkey && hotkey_text != nullptr)
		{
			hotkey_text->BlitElement();
		}

		UI_element::BlitElement();
	}
}

void Button::displayHotkey(bool display, _TTF_Font* font, SDL_Color color, bool atCenter)
{
	displayingHotkey = display;

	if (display = true && font != nullptr && Hotkey != 0)
	{
		if (hotkey_text != nullptr)
		{
			childs.clear();
			RELEASE(hotkey_text);
		}
		
		std::string text;
		text = (char)(Hotkey + 61);

		hotkey_text = new Text(text, 0, 0, font, color, nullptr);
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
}

SDL_Scancode Button::getHotkey() const
{
	return Hotkey;
}
