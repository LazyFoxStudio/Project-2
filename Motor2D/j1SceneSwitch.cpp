#include "j1SceneSwitch.h"
#include "j1Render.h"
#include "j1App.h"
#include "j1Gui.h"
#include "j1Audio.h"
#include "j1Map.h"
#include "j1EntityController.h"
#include "j1Window.h"
#include "SDL/include/SDL_render.h"



j1SceneSwitch::j1SceneSwitch()
{
	name.create("sceneswitch");
}


j1SceneSwitch::~j1SceneSwitch()
{
}

bool j1SceneSwitch::Awake()
{
	return true;
}

bool j1SceneSwitch::Start()
{
	screen.x = 0;
	screen.y = 0;
	screen.w = App->win->width;
	screen.h = App->win->height;
	return true;
}

bool j1SceneSwitch::Update(float dt)
{
	BROFILER_CATEGORY("SceneSwitchUpdate", Profiler::Color::AliceBlue);
	bool ret = true;

	if (current_step == fade_step::none)
		return ret;

	float normalized = MIN(1.0f, switchtimer.ReadSec() / fadetime);

	switch (current_step)
	{
		case fade_step::fade_to_black:
		{
			if (switchtimer.ReadSec() >= fadetime)
			{
				to_disable->Disable();
				App->gui->CleanUp();
				App->map->CleanUp();
				App->entitycontroller->DeleteEntities();
				App->gui->Start();
				to_enable->Enable();
				switchtimer.Start();
				if (App->sceneswitch->switchingFromSaveGame)
				{
					App->LoadGame();
				}
				current_step = fade_step::fade_from_black;
			}
		}break;

		case fade_step::fade_from_black:
		{
			normalized = 1.0f - normalized;

			if (switchtimer.ReadSec() >= fadetime)
			{
				current_step = fade_step::none;
			}
		}break;
	}

	SDL_SetRenderDrawColor(App->render->renderer, 0, 0, 0, (Uint8)(normalized * 255.0f));
	SDL_RenderFillRect(App->render->renderer, &screen);

	return ret;
}

bool j1SceneSwitch::SwitchScene(j1Module* SceneIn, j1Module* SceneOut)
{
	bool ret = false;

	if (current_step == fade_step::none)
	{
		current_step = fade_step::fade_to_black;
		switchtimer.Start();
		to_enable = SceneIn;
		to_disable = SceneOut;
		ret = true;
	}
	return true;
}

bool j1SceneSwitch::IsSwitching() const
{
	return (current_step != fade_step::none);
}
