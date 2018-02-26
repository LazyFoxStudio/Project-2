#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "p2List.h"
#include "SDL/include/SDL.h"
#include "UIElement.h"


struct WinElement
{
	WinElement(UIElement* element, iPoint relativePosition);
	UIElement* element;
	iPoint relativePosition;

};

class Window
{
public:
	Window();
	Window(SDL_Rect &collider, bool draggable = false);
	~Window();

	bool PreUpdate();
	WinElement* AddElementToWindow(UIElement* element, iPoint relativePosition);
	void moveElements(iPoint difference);

private:
	void HandleMovement();
	void HandleFocus();
	void FocusOnFirstInteractiveElement();
	void FindNextFocusableElement(p2List_item<WinElement*>* current);

public:
	bool active = true;
	bool draggable = false;
	bool hasFocus = false;
	p2List<WinElement*> children_list;
	iPoint mouseLastFrame;
	SDL_Rect collider;

private:

};

#endif

