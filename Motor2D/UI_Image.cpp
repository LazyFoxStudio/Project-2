#include "UI_Image.h"
#include "j1App.h"
#include "j1Render.h"
#include "Brofiler\Brofiler.h"

void Image::BlitElement()
{
	if (active)
	{
		BROFILER_CATEGORY("Image Blit", Profiler::Color::Beige);

		if (texture != App->gui->atlas)
			SDL_SetTextureAlphaMod(texture, App->gui->alpha_value);
		iPoint globalPos = calculateAbsolutePosition();
		if (border)
		{
			SDL_Rect border_rect = { globalPos.x - border_thickness / 2, globalPos.y - border_thickness / 2, section.w + border_thickness, section.h + border_thickness };
			App->render->DrawQuad(border_rect, border_color, true, use_camera, true);
		}
		App->render->Blit(texture, globalPos.x, globalPos.y, &section, use_camera, true, 1.0f, SDL_FLIP_NONE, 1.0f, rotation);

		UI_element::BlitElement();
	}
}

void Image::setBorder(bool border, Color color, int thickness)
{
	this->border = border;
	if (border)
	{
		border_color = color;
		border_thickness = thickness;
	}
}
