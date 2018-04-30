#include "j1App.h"
#include "UI_Window.h"
#include "j1App.h"
#include "j1Render.h"
#include "p2Defs.h"
#include "j1Gui.h"
#include "j1Render.h"
#include "Brofiler\Brofiler.h"

void Window::BlitElement()
{
	if (active)
	{
		BROFILER_CATEGORY("Window Blit", Profiler::Color::MediumPurple);

		SDL_SetTextureAlphaMod(texture, App->gui->alpha_value);
		iPoint globalPos = calculateAbsolutePosition();
		App->render->Blit(texture, globalPos.x, globalPos.y, &section, use_camera, true);

		UI_element::BlitElement();
	}
}
