#include "UI_CostDisplay.h"
#include "UI_Text.h"
#include "j1Fonts.h"
#include "UI_Image.h"
#include <string>





CostDisplay::CostDisplay(std::string entityname, int cost, resourceType resource)
{
	entity_name = new Text(entityname.c_str(), 400, 100, (*App->font->fonts.begin()), { 0,0,0,255 }, callback);
	cost_text = new Text(std::to_string(cost), 400, 200, (*App->font->fonts.begin()), { 0,0,0,255 }, callback);

	TextList.push_back(entity_name);
	Number.push_back(cost_text);

	Image* iconWood = new Image(icon_atlas, 400, 100, { 0,0,100,100 }, callback);
	switch (resource)
	{
	case NONE_NATURE:
		break;
	case WOOD:
		ResourceIcons.push_back(iconWood);
		break;
	case GOLD:
		break;
	case STONE:
		break;
	}
	BlitElement(true);
};


CostDisplay::~CostDisplay()
{
	
}

void CostDisplay::BlitElement(bool use_camera)
{
	for (std::list<Text*>::iterator it_t = TextList.begin(); it_t != TextList.end(); it_t++)
	{
		if ((*it_t)->active)
			(*it_t)->BlitElement(use_camera);
	}
	for (std::list<Text*>::iterator it_t = Number.begin(); it_t != Number.end(); it_t++)
	{
		if ((*it_t)->active)
			(*it_t)->BlitElement(use_camera);
	}
}
;

