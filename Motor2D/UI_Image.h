#ifndef __UI_IMAGE__
#define __UI_IMAGE__
#include "UI_element.h"
#include "Color.h"

struct SDL_Rect;

class Image : public UI_element
{
public:

	Image()
	{}

	Image(SDL_Texture* texture, int x, int y, SDL_Rect section, j1Module* callback) : UI_element(x, y, element_type::IMAGE, section, callback, texture)
	{}

	~Image()
	{}

	void BlitElement(bool use_camera = false);

	void setBorder(bool border, Color color = White, int thickness = 4);

public:

	bool border = false;
	Color border_color;
	int border_thickness=0;
};

#endif // !__UI_IMAGE__
