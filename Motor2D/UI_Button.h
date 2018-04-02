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

	/*Button(int x, int y, SDL_Texture* texture, SDL_Rect standby, SDL_Rect OnMouse, SDL_Rect standbyActive, SDL_Rect OnMouseActive, j1Module* callback) : UI_element(x, y, element_type::SWITCH, standby, callback, texture),
		OnMouse(OnMouse),
		OnClick(OnMouse),
		sectionActive(standbyActive),
		OnMouseActive(OnMouseActive)
	{}*/

	~Button()
	{}

	void BlitElement(bool use_camera = false);

public:

	SDL_Rect OnMouse = { 0,0,0,0 };
	SDL_Rect OnClick = { 0,0,0,0 };
	bool clicked = false;
};

#endif // !__UI_BUTTON__
