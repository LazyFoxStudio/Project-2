#ifndef __j1FONTS_H__
#define __j1FONTS_H__

#include "j1Module.h"
#include <list>

#define DEFAULT_FONT "fonts/BebasNeue-Regular.ttf"
#define DEFAULT_FONT_SIZE 27
#define FONTS_FOLDER "fonts/"

struct SDL_Texture;
struct SDL_Color;
struct _TTF_Font;

class j1Fonts : public j1Module
{
public:

	j1Fonts();

	// Destructor
	virtual ~j1Fonts();

	// Called before render is available
	bool Awake(pugi::xml_node&);

	// Called before quitting
	bool CleanUp();

	// Load Font
	_TTF_Font* const Load(const char* path, int size = 12);

	// Create a surface from text
	SDL_Texture* Print(const char* text, SDL_Color* color, _TTF_Font* font = NULL);

	bool CalcSize(const char* text, int& width, int& height, _TTF_Font* font = NULL) const;

	void setFontOutline(_TTF_Font* font, int outline);

	void CloseFont(_TTF_Font* font);

	_TTF_Font* getFont(int id);

public:

	std::list<_TTF_Font*>	fonts;
	_TTF_Font*				default_font = nullptr;
};


#endif // __j1FONTS_H__