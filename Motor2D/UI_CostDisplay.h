#ifndef __UI_COSTDISPLAY__
#define __UI_COSTDISPLAY__

#include "UI_element.h"


struct _TTF_Font;

class Image;
class Text;

class CostDisplay : public UI_element
{
public:

	CostDisplay(SDL_Texture* icon_atlas, std::string entityname, int cost, resourceType resource);

	~CostDisplay();

	void BlitElement(bool use_camera = false);

public:
	SDL_Texture* icon_atlas = nullptr;
	Text* entity_name = nullptr;

	Text* text_name = nullptr;
	Text* text_cost = nullptr;
	Image* icon_image = nullptr; 
	int display_cost = 0;
};



#endif

