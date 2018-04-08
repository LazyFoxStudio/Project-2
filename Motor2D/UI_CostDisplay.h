#ifndef __UI_COSTDISPLAY__
#define __UI_COSTDISPLAY__

#include "UI_element.h"


struct _TTF_Font;

class Image;
class Text;

class CostDisplay : public UI_element
{
public:

	CostDisplay(std::string entityname, int cost, resourceType resource);

	~CostDisplay();

public:
	SDL_Texture * icon_atlas = nullptr;
	Text* entity_name = nullptr;
	Text* cost_text = nullptr;
	void BlitElement(bool use_camera = true);
	std::list<Text*> TextList; //Constant but inactive
	std::list<Text*> Number; //Constant but inactive
	std::list<Image*> ResourceIcons;
};



#endif

