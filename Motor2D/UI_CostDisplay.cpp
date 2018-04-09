#include "UI_CostDisplay.h"
#include "UI_Text.h"
#include "j1Fonts.h"
#include "UI_Image.h"
#include <string>
#include "j1Scene.h"
#include "j1Render.h"

CostDisplay::CostDisplay(SDL_Texture* icon_atlas, std::string entityname, int cost, resourceType resource)
{
	localPosition = { 400, 70 };

	text_name = new Text(entityname.c_str(), 0, 0, (*App->font->fonts.begin()), { 255,255,255,255 }, callback);
	text_cost = new Text(std::to_string(cost), 50, 37, (*App->font->fonts.begin()), { 255,255,255,255 }, callback);
	display_cost = cost;

	icon_image = new Image(icon_atlas, 0, 35, {436,341,37,36 }, callback); //WOOD

	section = { 390, 65, (int)text_name->tex_width + 15, (int)(text_name->tex_height + icon_image->section.h + 15) };
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

	appendChild(text_name);
	appendChild(text_cost);
	appendChild(icon_image);
};


CostDisplay::~CostDisplay()
{
	std::list<UI_element*>::iterator it_e;
	it_e = childs.begin();
	while ((*it_e) != nullptr && it_e != childs.end())
	{
		RELEASE((*it_e));
		it_e++;
	}
	childs.clear();
}

void CostDisplay::BlitElement(bool use_camera)
{
	Color Dark_Grey = Color(75, 75, 75, 200);
	iPoint globalPosition = calculateAbsolutePosition();
	App->render->DrawQuad({globalPosition.x-10, globalPosition.y-5, section.w, section.h}, Dark_Grey, true, use_camera);

	if (display_cost > App->scene->wood)
	{
		text_cost->setColor({ 255, 0, 0,255 }); //red
	}
	else if (display_cost < App->scene->wood)
	{
		text_cost->setColor({ 0, 255, 0,255 }); //green
	}
	
	UI_element::BlitElement(use_camera);
};

