#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Window.h"

#include "SDL/include/SDL.h"


j1Window::j1Window() : j1Module()
{
	window = nullptr;
	screen_surface = nullptr;
	name = "window";
	pausable = false;
}

// Destructor
j1Window::~j1Window() {}

bool j1Window::Start()
{
	icon = IMG_Load(icon_window_path.c_str());
	SDL_SetWindowIcon(window, icon);
	SDL_FreeSurface(icon);
	icon = nullptr;

	return true;
}

// Called before render is available
bool j1Window::Awake(pugi::xml_node& config)
{
	LOG("Init SDL window & surface");

	if(SDL_Init(SDL_INIT_VIDEO) >= 0)
	{
		//Create window
		Uint32 flags = SDL_WINDOW_SHOWN;
		bool fullscreen = config.child("fullscreen").attribute("value").as_bool(false);
		bool borderless = config.child("borderless").attribute("value").as_bool(false);
		bool resizable = config.child("resizable").attribute("value").as_bool(false);
		bool fullscreen_window = config.child("fullscreen_window").attribute("value").as_bool(false);
		SDL_DisplayMode default_resolution;
		SDL_GetCurrentDisplayMode(0, &default_resolution);
		if (config.child("resolution").attribute("default").as_bool(false) == true)
		{
			SDL_DisplayMode default_resolution;
			SDL_GetCurrentDisplayMode(0, &default_resolution);
			width = default_resolution.w;
			height = default_resolution.h;
		}
		else
		{
			width = config.child("resolution").attribute("width").as_int(640);
			height = config.child("resolution").attribute("height").as_int(480);
		}
		scale = config.child("resolution").attribute("scale").as_int(1);

		if(fullscreen)				flags |= SDL_WINDOW_FULLSCREEN;
		else if(borderless)			flags |= SDL_WINDOW_BORDERLESS;
		else if(resizable)			flags |= SDL_WINDOW_RESIZABLE;
		else if(fullscreen_window)	flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

		window = SDL_CreateWindow(App->GetTitle(), 0, 35, width, height, flags);

		if(window) screen_surface = SDL_GetWindowSurface(window);
		else
		{
			LOG("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			return false;
		}
	}
	else
	{
		LOG("SDL_VIDEO could not initialize! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	icon_window_path = config.child("icon").attribute("path").as_string("not_found");

	return true;
}

// Called before quitting
bool j1Window::CleanUp()
{
	LOG("Destroying SDL window and quitting all SDL systems");

	//Destroy window
	if(window) SDL_DestroyWindow(window);
	//Quit SDL subsystems
	SDL_Quit();
	return true;
}

// Set new window title
void j1Window::SetTitle(const char* new_title)
{
	//title.create(new_title);
	SDL_SetWindowTitle(window, new_title);
}

void j1Window::GetWindowSize(uint& width, uint& height) const
{
	width = this->width;
	height = this->height;
}
