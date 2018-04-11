#ifndef __UI_COSTDISPLAY__
#define __UI_COSTDISPLAY__

#include "UI_element.h"

struct _TTF_Font;

class Image;
class Text;

struct resource_cost
{
	resource_cost(SDL_Texture* texture, int cost, SDL_Rect icon);

	Text* text_cost = nullptr;
	Image* resource_icon = nullptr;
	int cost_num = 0;
};

class CostDisplay : public UI_element
{
public:

	CostDisplay(SDL_Texture* texture, std::string entityname, int wood_cost = 0, int gold_cost = 0, int oil_cost = 0, int workers_cost = 0);

	~CostDisplay();

	void BlitElement(bool use_camera = false);

public:
	//SDL_Texture* icon_atlas = nullptr;

	Text* text_name = nullptr;

	resource_cost* wood = nullptr;
	resource_cost* gold = nullptr;
	resource_cost* oil = nullptr;
	resource_cost* workers = nullptr;
	uint resources_num = 0;
};



#endif

