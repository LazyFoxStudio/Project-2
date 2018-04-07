#ifndef __UI_ELEMENT__
#define __UI_ELEMENT__

#include "j1Module.h"
#include "p2Point.h"
#include "j1App.h"
#include "j1Textures.h"
#include "j1Gui.h"
#include "j1Input.h"
#include "j1Window.h"
#include "j1App.h"

struct SDL_Texture;

enum element_type
{
	TEXT,
	IMAGE,
	BUTTON,
	WINDOW,
	CHRONO,
	PROGRESSBAR,
	MENU
};

enum element_function
{
	NONE,

	NEW_GAME,
	CONTINUE,
	SETTINGS,
	CREDITS,
	EXIT,
	PAUSE,
	RESTART,
	RESTORE,
	BACK,
	CANCEL,
	APPLY,
	HOME,
	WEB,

	MOVE_FUNCTION,
	BUILD_BARRACKS_FUNCTION,
	BUILD_LUMBER_MILL_FUNCTION
};

enum element_state
{
	STANDBY,
	MOUSEOVER,
	CLICKED
};

class UI_element
{
public:

	UI_element()
	{}

	UI_element(int x, int y, element_type type, SDL_Rect section, j1Module* callback, SDL_Texture* texture = nullptr) :
		localPosition({ x, y }),
		Original_Pos({ x, y }),
		element_type(type),
		section(section),
		callback(callback),
		texture(texture)
	{}

	virtual ~UI_element();

	virtual void setOutlined(bool isOutlined)
	{}

	iPoint calculateAbsolutePosition() const;

	void appendChild(UI_element* child, bool center = false);

	virtual void BlitElement(bool use_camera = false)
	{
		BlitChilds();
		if (state == MOUSEOVER)
		{
			if (!blitPopUpInfo)
			{
				if (App->gui->hovering_element.ReadMs() > 600)
				{
					if (popUp != nullptr)
					{
						int x, y;
						App->input->GetMousePosition(x, y);
						int win_w = App->win->width;
						int win_h = App->win->height;
						if (x + popUp->section.w > win_w)
						{
							x -= ((x + popUp->section.w) - win_w);
						}
						x = x;
						y -= popUp->section.h;
						if (y + popUp->section.y > win_h)
						{
							y -= ((y + popUp->section.h) - win_h);
						}
						popUp->localPosition = { x, y };
					}
					blitPopUpInfo = true;
				}
			}
			else
				BlitHoverExtraEffect();
		}
	}

	virtual void BlitHoverExtraEffect()
	{}

	void BlitChilds();

	virtual UI_element* getMouseHoveringElement()
	{
		UI_element* ret = this;
		if (childs.size() > 0)
		{
			for (std::list<UI_element*>::iterator it_c = childs.begin(); it_c != childs.end(); it_c++)
				if (App->gui->checkMouseHovering((*it_c)) && (*it_c)->interactive)
					ret = (*it_c)->getMouseHoveringElement();
		}

		return ret;
	}

	void setDragable(bool horizontally, bool vertically);

	void setOriginalPos(int x, int y);

	//-1 to delete limit
	void setLimits(int right_limit, int left_limit, int top_limit, int bottom_limit);

	void Mouse_Drag();

	void Start_Drag();

	void End_Drag();

public:

	SDL_Texture* texture = nullptr;
	iPoint localPosition = { 0,0 };
	SDL_Rect section = { 0, 0, 0, 0 };
	element_type element_type;
	element_state state = STANDBY;
	element_function function = NONE;
	j1Module* callback = nullptr;
	UI_element* parent = nullptr;
	std::list<UI_element*> childs;
	UI_element* popUp = nullptr;
	bool hovering = false;
	bool moving = false;
	bool dragable = false;
	bool interactive = true;
	bool active = true;
	bool blitPopUpInfo = false;
	std::string PopUpInfo;

protected:
	iPoint Click_Pos{ 0,0 };
	iPoint Original_Pos{ 0, 0 };
	bool verticalMovement = false;
	bool horizontalMovement = false;
	int right_limit = -1;
	int left_limit = -1;
	int top_limit = -1;
	int bottom_limit = -1;
};

#endif // !__UI_ELEMENT__