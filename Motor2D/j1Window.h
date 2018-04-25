#ifndef __j1WINDOW_H__
#define __j1WINDOW_H__

#include "j1Module.h"
#include <string>
#include "SDL_image\include\SDL_image.h"

struct SDL_Window;
struct SDL_Surface;

class j1Window : public j1Module
{
public:

	j1Window();

	// Destructor
	virtual ~j1Window();

	bool Start();

	// Called before render is available
	bool Awake(pugi::xml_node&);

	// Called before quitting
	bool CleanUp();

	// Changae title
	void SetTitle(const char* new_title);

	// Retrive window size
	void GetWindowSize(uint& width, uint& height) const;

	// Retrieve window scale
	uint GetScale() const { return scale; };

public:
	//The window we'll be rendering to
	SDL_Window* window = nullptr;

	//The surface contained by the window
	SDL_Surface* screen_surface = nullptr;

	std::string	title="";
	int			width=0;
	int			height=0;
	int			scale=0;

	std::string icon_window_path = "";
	SDL_Surface* icon = nullptr;
};

#endif // __j1WINDOW_H__