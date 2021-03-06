#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Input.h"
#include "j1Textures.h"
#include "j1Pathfinding.h"
#include "j1Audio.h"
#include "j1Scene.h"
#include "j1Map.h"
#include "Color.h"
#include "j1EntityController.h"
#include <math.h>

j1Map::j1Map() { name = "map"; }

void j1Map::Draw()
{
	BROFILER_CATEGORY("Map draw", Profiler::Color::BlanchedAlmond);

	iPoint wCoord;
	iPoint starting_tile = App->map->WorldToMap(App->render->culling_camera.x, App->render->culling_camera.y);
	iPoint last_tile = { starting_tile.x + (App->render->culling_camera.w / data.tile_width), starting_tile.y + (App->render->culling_camera.h / data.tile_height) };

	if (starting_tile.x < 0) starting_tile.x = 0;
	if (starting_tile.y < 0) starting_tile.y = 0;

	for (uint b = 0; b < data.tilesets.size(); b++)
		for (uint i = starting_tile.y; i < last_tile.y; i++)
			for (uint j = starting_tile.x; j < last_tile.x; j++)
			{
				// TODO
				int tile_id = data.layers[0]->GetID(j, i);
				if (tile_id != 0)
				{
					TileSet* tileset = GetTilesetFromTileId(tile_id);
					SDL_Rect r = tileset->GetTileRect(tile_id);
					iPoint tileWorld = MapToWorld(j, i);

					App->render->Blit(tileset->texture, tileWorld.x, tileWorld.y, &r);
				}

				tile_id = data.layers[1]->GetID(j, i);
				if (tile_id > 0)
				{
					TileSet* tileset = GetTilesetFromTileId(tile_id);
					SDL_Rect r = tileset->GetTileRect(tile_id);
					iPoint tileWorld = MapToWorld(j, i);

					App->render->Blit(tileset->texture, tileWorld.x, tileWorld.y, &r);
				}

				if (debug)
				{
					if (!App->pathfinding->IsWalkable(iPoint{ (int)j,(int)i }))
					{
						iPoint tileWorld = MapToWorld(j, i);
						SDL_Rect debug_r = { tileWorld.x, tileWorld.y, data.tile_width, data.tile_height };
						App->render->DrawQuad(debug_r, Red);
					}
				}

			}
}


iPoint j1Map::MapToWorld(int x, int y) const
{
	iPoint ret;

	ret.x = x * data.tile_width;
	ret.y = y * data.tile_height;

	return ret;
}

iPoint j1Map::WorldToMap(int x, int y) const
{
	iPoint ret(0, 0);

	if (data.type == MAPTYPE_ORTHOGONAL)
	{
		ret.x = x / data.tile_width;
		ret.y = y / data.tile_height;
	}
	else if (data.type == MAPTYPE_ISOMETRIC)
	{

		float half_width = data.tile_width * 0.5f;
		float half_height = data.tile_height * 0.5f;
		ret.x = int((x / half_width + y / half_height) / 2) - 1;
		ret.y = int((y / half_height - (x / half_width)) / 2);
	}
	else
	{
		LOG("Unknown map type");
		ret.x = x; ret.y = y;
	}

	return ret;
}

bool j1Map::WalkabilityArea(int x, int y, int rows, int columns, bool modify, bool check_trees, bool check_mines)
{
	bool ret = true;
	if (App->pathfinding->map)
	{
		iPoint currentTile = { x,y };
		MapLayer* tree_layer = data.layers[0];
		if (check_trees || check_mines)
		{
			for (int i = 0; i < data.layers.size(); i++)
			{
				MapLayer* layer = data.layers[i];

				if (layer->name == "Resources")
				{
					tree_layer = layer;
					break;
				}
			}
		}

		//give me the layer
		for (int i = 0; i < data.layers.size(); i++)
		{
			MapLayer* layer = data.layers[i];

			if (layer->name == "Resources")
			{
				App->entitycontroller->CreateForest(data.layers[i]);
				break;
			}
		}

		if (!modify && !check_trees && !check_mines)
		{
			for (int j = 0; j < columns; j++)
			{
				for (int i = 0; i < rows; i++)
				{
					iPoint currentMapTile = WorldToMap(currentTile.x + j * data.tile_width, currentTile.y + i * data.tile_height);
					if (!App->pathfinding->IsWalkable(currentMapTile))
					{
						ret = false;
						break;
					}

				}
			}
		}
		else if (modify && !check_trees && !check_mines)
		{
			for (int j = 0; j < columns; j++)
			{
				for (int i = 0; i < rows; i++)
				{
					iPoint currentMapTile = WorldToMap(currentTile.x + j * data.tile_width, currentTile.y + i * data.tile_height);
					int tile = (currentMapTile.y*App->map->data.width) + currentMapTile.x;

					App->pathfinding->map[tile] = (App->pathfinding->map[tile] != INVALID_WALK_CODE ? INVALID_WALK_CODE : 0);

				}
			}
		}
		else if (check_trees || check_mines)
		{
			for (int j = 0; j < columns; j++)
			{
				for (int i = 0; i < rows; i++)
				{
					iPoint currentMapTile = WorldToMap(currentTile.x + j * data.tile_width, currentTile.y + i * data.tile_height);
					if ((tree_layer->GetID(currentMapTile.x, currentMapTile.y) == 373 || tree_layer->GetID(currentMapTile.x, currentMapTile.y) == 136 || tree_layer->GetID(currentMapTile.x, currentMapTile.y) == 137 ||
						tree_layer->GetID(currentMapTile.x, currentMapTile.y) == 134 || tree_layer->GetID(currentMapTile.x, currentMapTile.y) == 128 || tree_layer->GetID(currentMapTile.x, currentMapTile.y) == 107 ||
						tree_layer->GetID(currentMapTile.x, currentMapTile.y) == 131 || tree_layer->GetID(currentMapTile.x, currentMapTile.y) == 125 || tree_layer->GetID(currentMapTile.x, currentMapTile.y) == 108 ||
						tree_layer->GetID(currentMapTile.x, currentMapTile.y) == 111 && check_trees) || (IsMine(tree_layer->GetID(currentMapTile.x,currentMapTile.y))&& check_mines))
					{
						ret = false;
						if (check_trees)
						{
							break;
						}
						
					}
					else if(check_mines)
					{
						ret = true;
						break;
					}
				
				} 
				if (!ret && check_trees) break;
				else if (ret && check_mines) break;
			}

		}
	}

	return ret;
}

bool j1Map::IsMine(uint ID)
{
	return (ID==336 || ID==337||ID==338||ID==355||ID==356||ID==357||ID==374||ID==375||ID==376);
}

//Returns the rect of the specified tile.
SDL_Rect TileSet::GetTileRect(int id) const
{
	int relative_id = id - firstgid;
	SDL_Rect rect;
	rect.w = tile_width;
	rect.h = tile_height;
	rect.x = margin + ((rect.w + spacing) * (relative_id % num_tiles_width));
	rect.y = margin + ((rect.h + spacing) * (relative_id / num_tiles_width));
	return rect;
}

// Called before quitting
bool j1Map::CleanUp()
{
	LOG("Unloading map");

	for (int i = 0; i < data.tilesets.size(); i++)
	{
		SDL_DestroyTexture(data.tilesets[i]->texture);
		RELEASE(data.tilesets[i]);
	}
	data.tilesets.clear();

	for (int i = 0; i < data.layers.size(); i++)	RELEASE(data.layers[i]);
	data.layers.clear();

	// Clean up the pugui tree
	map_file.reset();

	return true;
}

// Load new map
bool j1Map::Load_map(const char* file_name)
{
	LOG("Loading map");

	pugi::xml_parse_result result = map_file.load_file(PATH(MAP_FOLDER, file_name));

	if (result)
	{
		if (LoadMapProperties()) {

			// Load all tilesets info ----------------------------------------------
			pugi::xml_node tileset;
			for (tileset = map_file.child("map").child("tileset"); tileset; tileset = tileset.next_sibling("tileset"))
			{
				TileSet* set = new TileSet();

				LoadTilesetDetails(tileset, set);
				if (!LoadTilesetImage(tileset, set))	return false;

				data.tilesets.push_back(set);
			}

			// Load layer info ----------------------------------------------
			pugi::xml_node layer;
			for (layer = map_file.child("map").child("layer"); layer; layer = layer.next_sibling("layer"))
			{
				MapLayer* set = new MapLayer();
				LoadLayer(layer, set);
				data.layers.push_back(set);
			}


			LOG("Successfully parsed map XML file: %s", file_name);
			LOG("width: %d height: %d", data.width, data.height);
			LOG("tile_width: %d tile_height: %d", data.tile_width, data.tile_height);

			for (int i = 0; i < data.tilesets.size(); i++)
			{
				TileSet* s = data.tilesets[i];
				LOG("Tileset ----");
				LOG("name: %s firstgid: %d", s->name.c_str(), s->firstgid);
				LOG("tile width: %d tile height: %d", s->tile_width, s->tile_height);
				LOG("spacing: %d margin: %d", s->spacing, s->margin);
			}

			for (int i = 0; i < data.layers.size(); i++)
			{
				MapLayer* l = data.layers[i];
				LOG("Layer ----");
				LOG("name: %s", l->name.c_str());
				LOG("layer width: %d layer height: %d", l->width, l->height);
			}

			App->render->cam_limit_x = data.width * data.tile_width - App->render->camera.w;
			App->render->cam_limit_y = data.height * data.tile_height - App->render->camera.h;

			return true;
		}
	}

	LOG("Could not load map xml file %s. pugi error: %s", file_name, result.description());
	return false;
}

// Load map general properties
bool j1Map::LoadMapProperties()
{
	if(pugi::xml_node map = map_file.child("map"))
	{
		data.width = map.attribute("width").as_int();
		data.height = map.attribute("height").as_int();
		data.tile_width = map.attribute("tilewidth").as_int();
		data.tile_height = map.attribute("tileheight").as_int();

		std::string orientation(map.attribute("orientation").as_string());

		if(orientation == "orthogonal")		data.type = MAPTYPE_ORTHOGONAL;
		else if(orientation == "isometric")	data.type = MAPTYPE_ISOMETRIC;
		else if(orientation == "staggered")	data.type = MAPTYPE_STAGGERED;
		else data.type = MAPTYPE_UNKNOWN;

		return true;
	}

	LOG("Error parsing map xml file: Cannot find 'map' tag.");
	return false;
}

void j1Map::LoadTilesetDetails(pugi::xml_node& tileset_node, TileSet* set)
{
	set->name = (tileset_node.attribute("name").as_string());
	set->firstgid = tileset_node.attribute("firstgid").as_int();
	set->tile_width = tileset_node.attribute("tilewidth").as_int();
	set->tile_height = tileset_node.attribute("tileheight").as_int();
	set->margin = tileset_node.attribute("margin").as_int();
	set->spacing = tileset_node.attribute("spacing").as_int();
	pugi::xml_node offset = tileset_node.child("tileoffset");

	if(offset != NULL)
	{
		set->offset_x = offset.attribute("x").as_int();
		set->offset_y = offset.attribute("y").as_int();
	}
	else set->offset_x = set->offset_y = 0; 

}

bool j1Map::LoadTilesetImage(pugi::xml_node& tileset_node, TileSet* set)
{

	if(pugi::xml_node image = tileset_node.child("image"))
	{
		set->texture = App->tex->Load(PATH(MAP_FOLDER, image.attribute("source").as_string()));
		int w, h;
		SDL_QueryTexture(set->texture, NULL, NULL, &w, &h);

		set->tex_width = image.attribute("width").as_int();
		set->tex_height = image.attribute("height").as_int();

		if(set->tex_width <= 0)		set->tex_width = w;
		if(set->tex_height <= 0)	set->tex_height = h;

		set->num_tiles_width = set->tex_width / set->tile_width;
		set->num_tiles_height = set->tex_height / set->tile_height;

		return true;
	}

	LOG("Error parsing tileset xml file: Cannot find 'image' tag.");
	return false;
}

void j1Map::LoadLayer(pugi::xml_node & node, MapLayer * layer)
{	
	layer->name = node.attribute("name").as_string();
	layer->width = node.attribute("width").as_uint();
	layer->height = node.attribute("height").as_uint();
	LoadProperties(node, layer->properties);
	layer->data = new uint[layer->width * layer->height];
	
	memset(layer->data, 0, sizeof(uint)*(layer->width * layer->height));

	pugi::xml_node layer_node;
	int i = 0;

	for(layer_node = node.child("data").child("tile"); layer_node; layer_node = layer_node.next_sibling("tile"))
		layer->data[i++] = layer_node.attribute("gid").as_uint(0);
}

int Properties::Get(const char* value, int default_value) const
{
	for (int i = 0; i < list.size(); i++)
		if (list[i]->name == value) return list[i]->value;

	return default_value;
}


bool j1Map::LoadProperties(pugi::xml_node& node, Properties& properties)
{
	if (pugi::xml_node data = node.child("properties"))
	{
		pugi::xml_node prop;

		for (prop = data.child("property"); prop; prop = prop.next_sibling("property"))
		{
			Properties::Property* p = new Properties::Property();

			p->name = prop.attribute("name").as_string();
			p->value = prop.attribute("value").as_int();

			properties.list.push_back(p);
		}

		return true;
	}

	return false;
}


MapLayer::~MapLayer()
{
	delete[] data;
}

TileSet* j1Map::GetTilesetFromTileId(int id) const
{
	TileSet* set = data.tilesets.front();

	for (int i = 0; i < data.tilesets.size(); i++)
		if (id < data.tilesets[i]->firstgid && i > 0) { set = data.tilesets[i - 1];  break; }

	return set;
}

bool j1Map::CreateWalkabilityMap(int& width, int& height, uchar** buffer) const
{
	BROFILER_CATEGORY("Creating Path Map", Profiler::Color::DarkGreen);

	for(int i = 0; i < data.layers.size(); i++)
	{
		MapLayer* layer = data.layers[i];

		if (layer->name == "Resources")
		{
			uchar* map = new uchar[layer->width*layer->height];
			memset(map, 0, layer->width*layer->height);

			for (int y = 0; y < data.height; ++y)
			{
				for (int x = 0; x < data.width; ++x)
				{
					int i = (y*layer->width) + x;
					int tile_id = layer->GetID(x, y);

					if (tile_id != 0)  map[i] = INVALID_WALK_CODE;
				}
			}

			*buffer = map;
			width = data.width;
			height = data.height;
			return true;
		}
	}

	return false;
}