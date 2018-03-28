#include "UI_Button.h"
#include "j1App.h"
#include "j1Render.h"
#include "Brofiler\Brofiler.h"

void Button::BlitElement(bool use_camera)
{
	if (active)
	{
		BROFILER_CATEGORY("Button Blit", Profiler::Color::DarkKhaki);
		iPoint globalPos = calculateAbsolutePosition();
		switch (state)
		{
		case STANDBY:
			if (!active)
				App->render->Blit(texture, globalPos.x, globalPos.y, &section, use_camera);
			else
				App->render->Blit(texture, globalPos.x, globalPos.y, &sectionActive, use_camera);
			break;
		case MOUSEOVER:
			if (!active)
				App->render->Blit(texture, globalPos.x, globalPos.y, &OnMouse, use_camera);
			else
				App->render->Blit(texture, globalPos.x, globalPos.y, &OnMouseActive, use_camera);
			break;
		case CLICKED:
			App->render->Blit(texture, globalPos.x, globalPos.y, &OnClick, use_camera);
			break;
		}

		BlitChilds();
	}
}
