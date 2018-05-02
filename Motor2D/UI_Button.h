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

	~Button()
	{}

	void BlitElement();
	void displayHotkey(bool display, _TTF_Font* font = nullptr);
	void setHotkey(SDL_Scancode hotkey);
	SDL_Scancode getHotkey() const;
	void updatedPosition();

public:

	SDL_Rect OnMouse = { 0,0,0,0 };
	SDL_Rect OnClick = { 0,0,0,0 };
	bool clicked = false;

private:

	bool displayingHotkey = false;
	SDL_Scancode Hotkey = SDL_SCANCODE_UNKNOWN;
	Text* hotkey_text = nullptr;
};

#endif // !__UI_BUTTON__
