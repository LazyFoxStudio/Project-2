#include "UI_CostDisplay.h"
#include "UI_Text.h"
#include "j1Fonts.h"
#include "UI_Image.h"
#include <string>
#include "j1Scene.h"

CostDisplay::CostDisplay(SDL_Texture* icon_atlas, std::string entityname, int cost, resourceType resource)
{
	text_name = new Text(entityname.c_str(), 400, 70, (*App->font->fonts.begin()), { 255,255,255,255 }, callback);
	text_cost = new Text(std::to_string(cost), 450, 100, (*App->font->fonts.begin()), { 255,255,255,255 }, callback);
	display_cost = cost;



	icon_image = new Image(icon_atlas, 400, 100, {436,341,37,36 }, callback); //WOOD
	switch (resource)
	{
	case NONE_NATURE:
		break;
	case WOOD:
		break;
	case GOLD:
		break;
	case STONE:
		break;
	}
};


CostDisplay::~CostDisplay()
{
	
}

void CostDisplay::BlitElement(bool use_camera)
{
	text_name->BlitElement(use_camera);
	text_cost->BlitElement(use_camera);
	icon_image->BlitElement(use_camera);
	if (display_cost > App->scene->wood)
	{
		text_cost->setColor({ 255, 0, 0,255 }); //red
	}
	else
	{
		text_cost->setColor({ 0, 255, 0,255 }); //red
	}
	
};

