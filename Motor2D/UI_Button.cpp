#include "UI_Button.h"
#include "j1App.h"
#include "j1Render.h"
#include "Brofiler\Brofiler.h"

void Button::BlitElement()
{
	BROFILER_CATEGORY("Button Blit", Profiler::Color::DarkKhaki);
	iPoint globalPos = calculateAbsolutePosition();
	switch (state)
	{
	case STANDBY:
		if (!active)
			App->render->Blit(texture, globalPos.x, globalPos.y, &section, false);
		else
			App->render->Blit(texture, globalPos.x, globalPos.y, &sectionActive, false);
		break;
	case MOUSEOVER:
		if (!active)
			App->render->Blit(texture, globalPos.x, globalPos.y, &OnMouse, false);
		else
			App->render->Blit(texture, globalPos.x, globalPos.y, &OnMouseActive, false);
		break;
	case CLICKED:
		App->render->Blit(texture, globalPos.x, globalPos.y, &OnClick, false);
		break;
	}

	BlitChilds();
}
