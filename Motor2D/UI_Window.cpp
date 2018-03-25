#include "j1App.h"
#include "UI_Window.h"
#include "j1App.h"
#include "j1Render.h"
#include "p2Defs.h"
#include "j1Gui.h"
#include "j1Render.h"
#include "Brofiler\Brofiler.h"

void Window::appendChild(int x, int y, UI_element * child)
{
	child->localPosition = { x, y };
	child->setOriginalPos(x, y);
	child->parent = this;
	content.push_back(child);
	
}

void Window::appendChildAtCenter(UI_element * child)
{
	iPoint child_pos(section.w / 2, section.h / 2);
	child_pos.x -= child->section.w / 2;
	child_pos.y -= child->section.h / 2;
	child->localPosition = { child_pos.x, child_pos.y };
	child->setOriginalPos(child_pos.x, child_pos.y);
	child->parent = this;
	content.push_back(child);
}

void Window::BlitElement()
{
	BROFILER_CATEGORY("Window Blit", Profiler::Color::MediumPurple) ;

	SDL_SetTextureAlphaMod(texture, App->gui->alpha_value);
	iPoint globalPos = calculateAbsolutePosition();
	App->render->Blit(texture, globalPos.x, globalPos.y, &section);

	for (std::list<UI_element*>::iterator it_e = content.begin(); it_e != content.end(); it_e++)
	{
		(*it_e)->BlitElement();
	}
}
