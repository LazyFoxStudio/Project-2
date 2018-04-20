#ifndef __MINIMAP_H__

#include "SDL\include\SDL.h"
#include "SDL_image\include\SDL_image.h"
#include <list>
#include "Color.h"

/* this struct has the 2 elements needed to represent a point on the minimap: 
the rectangle we want to represent on the minimap(x,y,w,h)
the color we want it to be seen(RGBA)*/
struct point
{
	SDL_Rect rect;// real map space that wants to be represented
	SDL_Color color;// color we want the point on the minimap
};

/*This struct saves one sprite we want to blit, 
it stores a pointer to a surface and the part we will take from the minimap*/
struct sprite
{
	SDL_Surface* sprite_surface = nullptr;//sprite or image to blit
	SDL_Rect section;
};

class Minimap
{
public:

	std::list<point> point_queue;
	std::list<sprite> sprite_queue;

	// position of the minimap on the window
	int window_position_x;
	int window_position_y;

	//width and height of the minimap on the window
	int width;
	int height;

private:

	SDL_Surface* base_image = nullptr;// the base texture we are using will be saved here

	//relation to the map it is representing
	float ratio_x = 1;
	float ratio_y = 1;

	//total map width and height
	int map_width;
	int map_height;

public:

	Minimap(const char* base_texture_path, int _window_position_x, int _window_position_y, int _width, int _height, int tex_width = -1, int tex_height = -1);

	~Minimap();

	//and this one draws it directly
	void DrawMinimap();
	
	//used to add a point to the map
	void Addpoint (SDL_Rect rect, Color color);

	// used to add a sprite to the map, the sprite must be loaded in whoever calls this function
	void Draw_Sprite(SDL_Surface* img, int x, int y);

	//transforms a coordinate clicked in the minimap into a real map one
	void Mouse_to_map(int& map_x, int& map_y);
};


#endif 
