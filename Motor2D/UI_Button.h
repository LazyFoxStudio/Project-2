#ifndef __UI_BUTTON__
#define __UI_BUTTON__

#include "UI_element.h"
#include "UI_Text.h"

class Button : public UI_element
{
public:

	Button()
	{}

	Button(int x, int y, SDL_Texture* texture, SDL_Rect standby, SDL_Rect OnMouse, SDL_Rect OnClick, j1Module* callback) : UI_element(x, y, element_type::BUTTON, standby, callback, texture),
		OnMouse(OnMouse),
		OnClick(OnClick)
	{}

	Button(int x, int y, SDL_Texture* texture, SDL_Rect standbyOff, SDL_Rect OnMouseOff, SDL_Rect standbyOn, SDL_Rect OnMouseOn, j1Module* callback) : UI_element(x, y, element_type::SWITCH, standbyOff, callback, texture),
		OnMouse(OnMouseOff),
		OnClick(standbyOn),
		OnMouseOn(OnMouseOn)
	{}

	~Button()
	{}

	void BlitElement();
	void displayHotkey(bool display, _TTF_Font* font = nullptr, SDL_Color color = { 255,255,255,255 }, bool atCenter = false);
	void setHotkey(SDL_Scancode hotkey);
	SDL_Scancode getHotkey() const;
	void setReadingHotkey();
	void cancelReadingHotkey();
	void setCooldown(int cooldown);
	void Used();

public:

	SDL_Rect OnMouse = { 0,0,0,0 };
	SDL_Rect OnClick = { 0,0,0,0 };
	SDL_Rect OnMouseOn = { 0,0,0,0 };
	bool clicked = false;
	bool switchOn = false;

private:

	bool displayingHotkey = false;
	SDL_Scancode Hotkey = SDL_SCANCODE_UNKNOWN;
	Text* hotkey_text = nullptr;

	Text* readingHotkeyLine1 = nullptr;
	Text* readingHotkeyLine2 = nullptr;

	bool hasCooldown = false;
	bool inCooldown = false;
	int cooldown = 0;
	j1Timer cooldown_timer;
};

#endif // !__UI_BUTTON__
