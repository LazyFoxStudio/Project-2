#include "UI_IngameMenu.h"
#include "j1App.h"
#include "j1Render.h"
#include "Brofiler\Brofiler.h"

void IngameMenu::newSelection()
{
}

void IngameMenu::BlitElement(bool use_camera)
{
	BROFILER_CATEGORY("In-game Menu Blit", Profiler::Color::Beige);

	//update minimap
	//update health bars

	iPoint globalPos = calculateAbsolutePosition();
	App->render->Blit(texture, globalPos.x, globalPos.y, &section, use_camera);

	//Blit icons
	//Blit health bars
	//Blit action butons
}
