#include "UI_IngameMenu.h"
#include "j1App.h"
#include "j1Render.h"
#include "Brofiler\Brofiler.h"
#include "UI_Window.h"
#include "UI_Image.h"
#include "UI_Text.h"
#include "UI_Button.h"

IngameMenu::IngameMenu(SDL_Texture * texture, int x, int y, SDL_Rect section, int minimap_posX, int minimap_posY, int firstIcon_posX, int firstIcon_posY, int icons_offsetX, int icons_offsetY, int stats_posX, int stats_posY, int firstButton_posX, int firstButton_posY, int buttons_offsetX, int buttons_offsetY, j1Module * callback) : UI_element(x, y, element_type::MENU, section, callback, texture),
firstIcon_pos({firstIcon_posX, firstIcon_posY}),
icons_offset({icons_offsetX, icons_offsetY}),
stats_pos({stats_posX, stats_posY}),
firstButton_pos({firstButton_posX, firstButton_posY}),
buttons_offset({buttons_offsetX, buttons_offsetY})
{
	window = new Window(texture, x, y, section, callback);
	//Create minimap
}

void IngameMenu::newSelection()
{
}

void IngameMenu::BlitElement(bool use_camera)
{
	BROFILER_CATEGORY("In-game Menu Blit", Profiler::Color::Beige);

	//update minimap
	//update health bars


	//Blit window
	window->BlitElement(use_camera);
	//Blit minimap
	//Blit icons
	//Blit health bars
	//Blit action butons
}
