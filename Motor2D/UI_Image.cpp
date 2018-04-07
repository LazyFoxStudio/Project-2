#include "UI_Image.h"
#include "j1App.h"
#include "j1Render.h"
#include "Brofiler\Brofiler.h"

void Image::BlitElement(bool use_camera)
{
	if (active)
	{
		BROFILER_CATEGORY("Image Blit", Profiler::Color::Beige);

		if (texture != App->gui->GetAtlas())
			SDL_SetTextureAlphaMod(texture, App->gui->alpha_value);
		iPoint globalPos = calculateAbsolutePosition();
		if (border)
		{
			SDL_Rect border_rect = { globalPos.x - border_thickness / 2, globalPos.y - border_thickness / 2, section.w + border_thickness, section.h + border_thickness };
			App->render->DrawQuad(border_rect, border_color, true, use_camera);
		}
		App->render->Blit(texture, globalPos.x, globalPos.y, &section, use_camera);

		UI_element::BlitElement(use_camera);
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
