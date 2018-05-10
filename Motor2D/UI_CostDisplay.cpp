#include "UI_CostDisplay.h"
#include "UI_Text.h"
#include "j1Fonts.h"
#include "UI_Image.h"
#include <string>
#include "j1Scene.h"
#include "j1Render.h"

CostDisplay::CostDisplay(SDL_Texture* texture, std::string entityname, int wood_cost, int gold_cost, int oil_cost, int workers_cost): UI_element(0,0,COSTDISPLAY, {0,0,0,0}, nullptr, texture)
{
	text_name = new Text(entityname.c_str(), 0, 0, (*App->font->fonts.begin()), { 255,255,255,255 }, callback);

	SDL_Rect wood_icon = { 436,341,37,36 };
	SDL_Rect gold_icon = { 619,341,37,36 };
	SDL_Rect oil_icon = { 0,0,0,0 };
	SDL_Rect workers_icon = { 796,341,37,36 };

	if (wood_cost > 0)
	{
		wood = new resource_cost(texture, wood_cost, wood_icon);
		appendChild(wood->text_cost);
		appendChild(wood->resource_icon);
		resources_num++;
	}
	if (gold_cost > 0)
	{
		gold = new resource_cost(texture, gold_cost, gold_icon);
		appendChild(gold->text_cost);
		appendChild(gold->resource_icon);
		resources_num++;
	}
	if (oil_cost > 0)
	{
		oil = new resource_cost(texture, oil_cost, oil_icon);
		appendChild(oil->text_cost);
		appendChild(oil->resource_icon);
		resources_num++;
	}
	if (workers_cost > 0)
	{
		workers = new resource_cost(texture, workers_cost, workers_icon);
		appendChild(workers->text_cost);
		appendChild(workers->resource_icon);
		resources_num++;
	}

	section = { 0, 0, (int)text_name->tex_width + 15, (int)(text_name->tex_height + (workers_icon.h*resources_num) + 15) };

	appendChild(text_name);
};


CostDisplay::~CostDisplay()
{
	std::list<UI_element*>::iterator it_c;
	it_c = childs.begin();
	while (it_c != childs.end())
	{
		RELEASE((*it_c));
		it_c++;
	}
	childs.clear();
}

void CostDisplay::BlitElement()
{
	Color Dark_Grey = Color(75, 75, 75, 200);
	iPoint globalPosition = calculateAbsolutePosition();
	App->render->DrawQuad({ globalPosition.x - 10, globalPosition.y - 5, section.w, section.h }, Dark_Grey, true, use_camera, true);

	int offsetY = 36;

	if (wood != nullptr)
	{
		if(wood->cost_num > App->scene->wood)
			wood->text_cost->setColor({ 255, 0, 0,255 });
		else
			wood->text_cost->setColor({ 0, 255, 0,255 });

		wood->resource_icon->localPosition.y = offsetY;
		wood->text_cost->localPosition.y = offsetY;
		offsetY += 36;
	}
	if (gold != nullptr)
	{
		if (gold->cost_num > App->scene->gold)
			gold->text_cost->setColor({ 255, 0, 0,255 });
		else
			gold->text_cost->setColor({ 0, 255, 0,255 });

		gold->resource_icon->localPosition.y = offsetY;
		gold->text_cost->localPosition.y = offsetY;
		offsetY += 36;
	}
	if (oil != nullptr)
	{
		if (oil->cost_num > App->scene->oil)
			oil->text_cost->setColor({ 255, 0, 0,255 });
		else
			oil->text_cost->setColor({ 0, 255, 0,255 });

		oil->resource_icon->localPosition.y = offsetY;
		oil->text_cost->localPosition.y = offsetY;
		offsetY += 36;
	}
	if (workers != nullptr)
	{
		if (workers->cost_num > App->scene->inactive_workers_int)
			workers->text_cost->setColor({ 255, 0, 0,255 });
		else
			workers->text_cost->setColor({ 0, 255, 0,255 });

		workers->resource_icon->localPosition.y = offsetY;
		workers->text_cost->localPosition.y = offsetY;
		offsetY += 36;
	}
	
	UI_element::BlitElement();
};

resource_cost::resource_cost(SDL_Texture * texture, int cost, SDL_Rect icon)
{
	text_cost = new Text(std::to_string(cost), 50, 36, (*App->font->fonts.begin()), { 255,255,255,255 }, nullptr);
	resource_icon = new Image(texture, 0, 36, icon, nullptr);
	cost_num = cost;
}
