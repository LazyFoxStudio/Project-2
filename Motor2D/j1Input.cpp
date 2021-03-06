#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Input.h"
#include "j1Window.h"
#include "j1Gui.h"
#include "UI_Button.h"
#include "UI_WarningMessages.h"
#include "j1Audio.h"
#include "SDL/include/SDL.h"

#define MAX_KEYS 300

j1Input::j1Input() : j1Module()
{
	name = "input";
	pausable = false;

	windowEvents[WE_QUIT] = false;
	windowEvents[WE_HIDE] = false;
	windowEvents[WE_SHOW] = false;
	windowEvents[WE_COUNT] = false;

	keyboard = new j1KeyState[MAX_KEYS];
	memset(keyboard, KEY_IDLE, sizeof(j1KeyState) * MAX_KEYS);
	memset(mouse_buttons, KEY_IDLE, sizeof(j1KeyState) * NUM_MOUSE_BUTTONS);
}

// Destructor
j1Input::~j1Input()
{
	delete[] keyboard;
}

// Called before render is available
bool j1Input::Awake(pugi::xml_node& config)
{
	LOG("Init SDL input event system");
	SDL_Init(0);

	if(SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
	{
		LOG("SDL_EVENTS could not initialize! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	return true;
}

// Called before the first frame
bool j1Input::Start()
{
	SDL_StopTextInput();
	return true;
}

// Called each loop iteration
bool j1Input::PreUpdate()
{
	BROFILER_CATEGORY("input_preupdate", Profiler::Color::BlanchedAlmond);
	static SDL_Event event;

		wasKeyDown = SDL_SCANCODE_UNKNOWN;
		const Uint8* keys = SDL_GetKeyboardState(NULL);

		for (int i = 0; i < MAX_KEYS; ++i)
		{
			if (!console_input)
			{
				if (keys[i] == 1)
				{
					if (keyboard[i] == KEY_IDLE) { keyboard[i] = KEY_DOWN; wasKeyDown = (SDL_Scancode)i; }
					else						keyboard[i] = KEY_REPEAT;
				}
				else
				{
					if (keyboard[i] == KEY_REPEAT || keyboard[i] == KEY_DOWN)	keyboard[i] = KEY_UP;
					else														keyboard[i] = KEY_IDLE;
				}
			}				
			else if (i == SDL_SCANCODE_LCTRL || i == SDL_SCANCODE_C || i == SDL_SCANCODE_HOME || i == SDL_SCANCODE_HOME
				|| i == SDL_SCANCODE_END || i == SDL_SCANCODE_LEFT || i == SDL_SCANCODE_RIGHT || i == SDL_SCANCODE_BACKSPACE
				|| i == SDL_SCANCODE_DELETE || i == SDL_SCANCODE_RETURN || i == SDL_SCANCODE_UP || i == SDL_SCANCODE_DOWN)
			{
				if (keys[i] == 1)
				{
					if (keyboard[i] == KEY_IDLE) { keyboard[i] = KEY_DOWN; wasKeyDown = (SDL_Scancode)i; }
					else						keyboard[i] = KEY_REPEAT;
				}
				else
				{
					if (keyboard[i] == KEY_REPEAT || keyboard[i] == KEY_DOWN)	keyboard[i] = KEY_UP;
					else														keyboard[i] = KEY_IDLE;
				}
			}
		}
	
	for(int i = 0; i < NUM_MOUSE_BUTTONS; ++i)
	{
		if(mouse_buttons[i] == KEY_DOWN) mouse_buttons[i] = KEY_REPEAT;
		if(mouse_buttons[i] == KEY_UP) mouse_buttons[i] = KEY_IDLE;
	}

	while(SDL_PollEvent(&event) != 0)
	{
		switch(event.type)
		{
			case SDL_QUIT:
				windowEvents[WE_QUIT] = true;
			break;
			
			case SDL_TEXTINPUT:
				text_buffer += event.text.text;
			break;

			case SDL_WINDOWEVENT:
				switch(event.window.event)
				{
					//case SDL_WINDOWEVENT_LEAVE:
					case SDL_WINDOWEVENT_HIDDEN:
					case SDL_WINDOWEVENT_MINIMIZED:
					case SDL_WINDOWEVENT_FOCUS_LOST:
					windowEvents[WE_HIDE] = true;
					break;

					//case SDL_WINDOWEVENT_ENTER:
					case SDL_WINDOWEVENT_SHOWN:
					case SDL_WINDOWEVENT_FOCUS_GAINED:
					case SDL_WINDOWEVENT_MAXIMIZED:
					case SDL_WINDOWEVENT_RESTORED:
					windowEvents[WE_SHOW] = true;
					break;
				}
			break;

			case SDL_MOUSEBUTTONDOWN:
				mouse_buttons[event.button.button - 1] = KEY_DOWN;
				//LOG("Mouse button %d down", event.button.button-1);
			break;

			case SDL_MOUSEBUTTONUP:
				mouse_buttons[event.button.button - 1] = KEY_UP;
				//LOG("Mouse button %d up", event.button.button-1);
			break;

			case SDL_MOUSEMOTION:
				int scale = App->win->GetScale();
				mouse_motion_x = event.motion.xrel / scale;
				mouse_motion_y = event.motion.yrel / scale;
				mouse_x = event.motion.x / scale;
				mouse_y = event.motion.y / scale;
				//LOG("Mouse motion x %d y %d", mouse_motion_x, mouse_motion_y);
			break;
		}
	}

	return true;
}

bool j1Input::Update(float dt)
{
	if (assignNewHotkey && wasKeyDown != SDL_SCANCODE_UNKNOWN && wasKeyDown != SDL_SCANCODE_ESCAPE)
	{
		if (wasKeyDown >= SDL_SCANCODE_A && wasKeyDown <= SDL_SCANCODE_Z)
		{
			App->gui->assignActionButtonHotkey(buttonId, wasKeyDown);
			App->gui->GetElement(TEXT, 44)->active = false;

			assignNewHotkey = false;
		}
		else
		{
			App->gui->GetElement(TEXT, 44)->active = true;
			App->audio->PlayFx(SFXList::SFX_CANTDOTHAT);
		}
	}

	return true;
}

// Called before quitting
bool j1Input::CleanUp()
{
	LOG("Quitting SDL event subsystem");
	SDL_QuitSubSystem(SDL_INIT_EVENTS);
	return true;
}

// ---------
bool j1Input::GetWindowEvent(j1EventWindow ev)
{
	return windowEvents[ev];
}

void j1Input::GetMousePosition(int& x, int& y)
{
	x = mouse_x;
	y = mouse_y;
}

void j1Input::GetMouseMotion(int& x, int& y)
{
	x = mouse_motion_x;
	y = mouse_motion_y;
}

void j1Input::readHotkey(uint buttonId)
{
	if (!(assignNewHotkey && this->buttonId == buttonId))
	{
		if (assignNewHotkey)
		{
			Button* prevButton = App->gui->getButtonbyId(this->buttonId);
			prevButton->cancelReadingHotkey();
		}
		this->buttonId = buttonId;
		assignNewHotkey = true;
		Button* button = App->gui->getButtonbyId(buttonId);
		if (button != nullptr)
			button->setReadingHotkey();
	}
	else
	{
		assignNewHotkey = false;
		Button* prevButton = App->gui->getButtonbyId(this->buttonId);
		prevButton->cancelReadingHotkey();
	}
}

bool j1Input::isReadingHotkey() const
{
	return assignNewHotkey;
}

void j1Input::stopReadingHotkey()
{
	assignNewHotkey = false;
}

void j1Input::StartTextInput()
{
	SDL_StartTextInput();
	text_buffer.clear();
	console_input = true;
}

void j1Input::StopTextInput()
{
	SDL_StopTextInput();
	text_buffer.clear();
	console_input = false;
}