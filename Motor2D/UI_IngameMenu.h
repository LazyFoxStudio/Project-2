#ifndef __UI_INGAMEMENU__
#define __UI_INGAMEMENU__

#include "UI_element.h"

class Minimap;
class Image;

class IngameMenu : public UI_element
{
public:

	IngameMenu()
	{}

	IngameMenu(int x, int y, );

public:

	Minimap* minimap;
	Image* background;
	

};

#endif // !__UI_INGAMEMENU__