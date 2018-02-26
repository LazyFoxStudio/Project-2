#ifndef __j1MAP_H__
#define __j1MAP_H__

#include "PugiXml/src/pugixml.hpp"
#include "p2List.h"
#include "p2Point.h"
#include "j1Module.h"

struct SDL_Texture;
// ----------------------------------------------------
struct Properties
{
	struct Property
	{
		p2SString name;
		int value;
	};

	~Properties()
	{
		p2List_item<Property*>* item;
		item = list.start;

		while (item != NULL)
		{
			RELEASE(item->data);
			item = item->next;
		}

		list.clear();
	}

	int Get(const char* name, int default_value = 0) const;

	p2List<Property*>	list;
};
struct MapLayer
{
	p2SString name;
	uint width = 0;
	uint height = 0;
	uint* data = nullptr;
	Properties	properties;

	~MapLayer();
	inline uint GetID(uint x, uint y) const
	{
		return data[(y*width) + x];
	}
};

struct TileSet
{
	SDL_Rect GetTileRect(int id) const;

	p2SString			name;
	int					firstgid;
	int					margin;
	int					spacing;
	int					tile_width;
	int					tile_height;
	SDL_Texture*		texture;
	int					tex_width;
	int					tex_height;
	int					num_tiles_width;
	int					num_tiles_height;
	int					offset_x;
	int					offset_y;
};

enum MapTypes
{
	MAPTYPE_UNKNOWN = 0,
	MAPTYPE_ORTHOGONAL,
	MAPTYPE_ISOMETRIC,
	MAPTYPE_STAGGERED
};
// ----------------------------------------------------
struct MapData
{
	int						width;
	int						height;
	int						tile_width;
	int						tile_height;
	SDL_Color				background_color;
	MapTypes				type;
	p2List<TileSet*>		tilesets;
	p2List<MapLayer*>		layers;
};
// ----------------------------------------------------
class j1Map : public j1Module
{
//-------------Functions-----------------
public:

	j1Map();

	// Destructor
	virtual ~j1Map();

	// Called before render is available
	bool Awake(pugi::xml_node& conf);

	// Called each loop iteration
	void Draw();

	void DebugDraw();
	// Called before quitting
	bool CleanUp();

	// Load new map
	bool Load_map(const char* path);

	//Method that translates x,y coordinates from map positions to world positions
	iPoint MapToWorld(int x, int y) const;

	iPoint j1Map::WorldToMap(int x, int y) const;

	bool j1Map::CreateWalkabilityMap(int& width, int& height, uchar** buffer) const;
private:

	bool LoadMap();
	bool LoadTilesetDetails(pugi::xml_node& tileset_node, TileSet* set);
	bool LoadTilesetImage(pugi::xml_node& tileset_node, TileSet* set);
	bool LoadLayer(pugi::xml_node& node, MapLayer* layer);
	bool LoadProperties(pugi::xml_node& node, Properties& properties);

	TileSet* GetTilesetFromTileId(int id) const;

//-------------Variables-----------------
public:

	MapData				data;
	bool				debug = false;

private:

	pugi::xml_document	map_file;
	p2SString			folder;
	bool				map_loaded;
	
};

#endif // __j1MAP_H__