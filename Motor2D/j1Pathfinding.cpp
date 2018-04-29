#include "p2Log.h"
#include "j1App.h"
#include "j1PathFinding.h"
#include "j1EntityController.h"
#include "j1Map.h"
#include "SDL/include/SDL.h"

j1PathFinding::j1PathFinding() : j1Module(), map(nullptr),width(0), height(0)
{
	name = "pathfinding";
}

// Called before quitting
bool j1PathFinding::CleanUp()
{
	LOG("Freeing pathfinding library");

	last_path.clear();
	path_pool.clear();
	RELEASE_ARRAY(map);
	return true;
}

bool j1PathFinding::PostUpdate()
{
	BROFILER_CATEGORY("pf_postupdate", Profiler::Color::BlanchedAlmond);
	if (!path_pool.empty())
	{
		for (std::list<PathProcessor*>::iterator it = path_pool.begin(); it != path_pool.end(); it++)
			if ((*it)->flow_field->finished) { RELEASE(*it);  path_pool.erase(it); it--; }

		timer.Start();
		for (std::list<PathProcessor*>::iterator it = path_pool.begin(); it != path_pool.end(); it++)
		{
			if (!(*it)->ProcessFlowField(timer)) break;
		}
	}
	return true;
}

// Sets up the walkability map
void j1PathFinding::SetMap(uint width, uint height, uchar* data)
{
	this->width = width;
	this->height = height;

	RELEASE_ARRAY(map);
	map = new uchar[width*height];
	memcpy(map, data, width*height);
}

// Utility: return true if pos is inside the map boundaries
bool j1PathFinding::CheckBoundaries(const iPoint& pos) const
{
	return (pos.x >= 0 && pos.x < (int)width &&
			pos.y >= 0 && pos.y < (int)height);
}

// Utility: returns true is the tile is walkable
bool j1PathFinding::IsWalkable(const iPoint& pos) const
{
	return GetTileAt(pos) != INVALID_WALK_CODE;
}

// Utility: return the walkability value of a tile
uchar j1PathFinding::GetTileAt(const iPoint& pos) const
{
	if(CheckBoundaries(pos))
		return map[(pos.y*width) + pos.x];

	return INVALID_WALK_CODE;
}

// To request all tiles involved in the last generated path
const std::list<iPoint>* j1PathFinding::GetLastPath() const
{
	return &last_path;
}

// PathList ------------------------------------------------------------------------
// Looks for a node in this list and returns it's list node or NULL
// ---------------------------------------------------------------------------------
std::list<PathNode>::iterator PathList::Find(const iPoint& point)
{
	for (std::list<PathNode>::iterator it = list.begin(); it != list.end(); it++)
		if ((*it).pos == point) return it;

	return list.end();
}

// PathList ------------------------------------------------------------------------
// Returns the Pathnode with lowest score in this list or NULL if empty
// ---------------------------------------------------------------------------------
PathNode PathList::GetNodeLowestScore() const
{
	int min = 65535;
	const PathNode* node = nullptr;

	for (std::list<PathNode>::const_reverse_iterator it = list.rbegin(); it != list.rend(); it++)
	{
		if ((*it).Score() < min)
		{
			min = (*it).Score();
			node = &(*it);
		}
	}

	return (*node);
}

// PathNode -------------------------------------------------------------------------
// Convenient constructors
// ----------------------------------------------------------------------------------
PathNode::PathNode() : g(-1), h(-1), pos(-1, -1), parent(NULL)
{}

PathNode::PathNode(int g, int h, const iPoint& pos, PathNode* parent) : g(g), h(h), pos(pos), parent(parent)
{}

PathNode::PathNode(const PathNode& node) : g(node.g), h(node.h), pos(node.pos), parent(node.parent)
{}

// PathNode -------------------------------------------------------------------------
// Fills a list (PathList) of all valid adjacent pathnodes
// ----------------------------------------------------------------------------------
uint PathNode::FindWalkableAdjacents(PathList& list_to_fill, PathNode* parent)
{
	iPoint cell;

	cell.create(pos.x + 1, pos.y);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.list.push_back(PathNode(-1, -1, cell, parent));

	cell.create(pos.x, pos.y + 1);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.list.push_back(PathNode(-1, -1, cell, parent));

	cell.create(pos.x - 1, pos.y);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.list.push_back(PathNode(-1, -1, cell, parent));

	cell.create(pos.x, pos.y - 1);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.list.push_back(PathNode(-1, -1, cell, parent));


	return list_to_fill.list.size();
}

bool j1PathFinding::GatherWalkableAdjacents(iPoint map_pos, int count, std::vector<iPoint>& adjacents, int max_distance)
{
	if (!max_distance) max_distance = MAX_ADJACENT_DIST;

	iPoint cell;
	int radius = 1;
	uint found = 0;

	SDL_Rect r = { 0,0, App->map->data.tile_width, App->map->data.tile_height };

	while (radius < max_distance)
	{
		for (int i = -radius; i <= radius; i++)
			for (int j = -radius; j <= radius; j++)
				if (std::abs(i) == radius || std::abs(j) == radius)
				{
					cell.create(map_pos.x + i, map_pos.y + j);
					if (App->pathfinding->IsWalkable(cell))
					{
						iPoint world_p = App->map->MapToWorld(cell.x, cell.y);
						r.x = world_p.x; r.y = world_p.y;

						std::vector<Entity*> collisions;
						App->entitycontroller->CheckCollidingWith(r, collisions);

						if (collisions.empty())
						{
							adjacents.push_back(cell);
							found++;
							if (found == count) return true;
						}
					}
				}
		radius++;
	}
	return false;
}

iPoint j1PathFinding::FirstWalkableAdjacent(iPoint map_pos, int max_distance)
{
	if (!max_distance) max_distance = MAX_ADJACENT_DIST;

	iPoint ret;
	int radius = 1;

	while (radius < max_distance)
	{
		for (int i = -radius; i <= radius; i++)
			for (int j = -radius; j <= radius; j++)
				if (std::abs(i) == radius || j == std::abs(j))
				{
					ret.create(map_pos.x + i, map_pos.y + j);
					if (App->pathfinding->IsWalkable(ret))
						return ret;
				}
		radius++;
	}
	return { -1,-1 };
}

// PathNode -------------------------------------------------------------------------
// Calculates this tile score
// ----------------------------------------------------------------------------------
int PathNode::Score() const
{
	return g + h;
}

// PathNode -------------------------------------------------------------------------
// Calculate the F for a specific destination tile
// ----------------------------------------------------------------------------------
int PathNode::CalculateF(const iPoint& destination)
{
	g = parent->g + 1;
	h = pos.DistanceNoSqrt(destination);

	return g + h;
}

// ----------------------------------------------------------------------------------
// Actual A* algorithm: return number of steps in the creation of the path or -1 ----
// ----------------------------------------------------------------------------------
int j1PathFinding::CreatePath(const iPoint& origin, iPoint& destination)
{
	BROFILER_CATEGORY("Pathfinding", Profiler::Color::Magenta);
	last_path.clear();
	int ret = -1;

	if (!IsWalkable(destination))
	{
		destination = FirstWalkableAdjacent(destination);
		if (destination.x == -1)
			return ret;
	}

	PathList open;
	PathList closed;

	PathNode originNode(0, origin.DistanceNoSqrt(destination), origin, nullptr);

	open.list.push_back(originNode);

	while (open.list.size())
	{
		ret++;
		if (ret > 400)
			return -1;

		PathNode lowestScoreNode = open.GetNodeLowestScore();

		closed.list.push_back(lowestScoreNode);
		open.list.remove(lowestScoreNode);

		if (lowestScoreNode.pos == destination)
		{
			PathNode current;
			for (current = lowestScoreNode; current.parent; current = *current.parent)
				last_path.push_front(current.pos);

			break;
		}

		PathList neighbors;
		lowestScoreNode.FindWalkableAdjacents(neighbors, &(*closed.Find(lowestScoreNode.pos)));

		for (std::list<PathNode>::iterator current = neighbors.list.begin(); current != neighbors.list.end(); current++)
		{
			if (closed.Find((*current).pos) == closed.list.end())
			{
				(*current).CalculateF(destination);

				if (open.Find((*current).pos) == open.list.end())		open.list.push_back(*current);
				else
				{
					std::list<PathNode>::iterator prev_node = open.Find((*current).pos);
					if ((*prev_node).Score() > (*current).Score())
					{
						open.list.erase(prev_node);
						open.list.push_back(*current);
					}
				}
			}
		}
	}

	return ret;
}


int FieldNode::CalculateScore(iPoint goal)
{
	h = position.DistanceManhattan(goal);
	g = parent->g + 1;
	return (score = h + g);
}

void FieldNode::getWalkableAdjacents(std::vector<FieldNode>& list_to_fill, FieldNode* parent)
{
	FieldNode new_adjacent;
	new_adjacent.parent = parent;

	new_adjacent.position.create(position.x + 1, position.y);
	if (App->pathfinding->IsWalkable(new_adjacent.position))
		list_to_fill.push_back(new_adjacent);

	new_adjacent.position.create(position.x, position.y + 1);
	if (App->pathfinding->IsWalkable(new_adjacent.position))
		list_to_fill.push_back(new_adjacent);

	new_adjacent.position.create(position.x - 1, position.y);
	if (App->pathfinding->IsWalkable(new_adjacent.position))
		list_to_fill.push_back(new_adjacent);

	new_adjacent.position.create(position.x, position.y - 1);
	if (App->pathfinding->IsWalkable(new_adjacent.position))
		list_to_fill.push_back(new_adjacent);
}

FlowField::FlowField(uint width, uint height, int init_to) : width(width), height(height)
{
	field = new FieldNode*[width];
	this->width = width;
	this->height = height;

	for (int x = 0; x < width; x++)
	{
		field[x] = new FieldNode[height];
		for (int y = 0; y < height; y++)
		{
			field[x][y].position = iPoint{ x, y };
			field[x][y].score = init_to;
		}
	}
}

FlowField::~FlowField()
{
	if (field) {
		for (int x = 0; x < width; x++)
			RELEASE_ARRAY(field[x]);
	}

	RELEASE_ARRAY(field);
}

void FlowField::ClearTo(int value)
{
	for (int x = 0; x < width; x++)
		for (int y = 0; y < height; y++)
		{
			field[x][y].parent = nullptr;
			field[x][y].score = value;
		}
	
}

FieldNode* FlowField::getNodeAt(iPoint p) 
{
	if (App->pathfinding->CheckBoundaries(p))
		return &field[p.x][p.y];
	else
		return &field[0][0];
}

FlowField* j1PathFinding::RequestFlowField(iPoint destination)
{
	PathProcessor* pp = new PathProcessor(destination);
	App->pathfinding->path_pool.push_back(pp);
	return pp->flow_field;
}

PathProcessor::PathProcessor(iPoint destination) : destination(destination)
{ 
	flow_field = new FlowField(App->map->data.width, App->map->data.height);
}

bool PathProcessor::ProcessFlowField(j1Timer& timer)
{
	BROFILER_CATEGORY("Process Flow Field", Profiler::Color::SeaGreen);
	switch (flow_field->stage)
	{
	case REQUESTED:

		if (!App->pathfinding->IsWalkable(destination))
			destination = App->pathfinding->FirstWalkableAdjacent(destination);

		if (!App->pathfinding->IsWalkable(destination))		flow_field->stage = FAILED;
		else
		{
			flow_field->getNodeAt(destination)->score = 0;
			open.push_back(*flow_field->getNodeAt(destination));
			flow_field->stage = PROCESSING;
		}

		break;

	case PROCESSING:

		while (!open.empty())
		{
			if (timer.Read() > ((float)App->framerate * PF_MAX_FRAMETIME))
				return false;

			std::vector<FieldNode> adjacents;
			FieldNode current_tile = open.front();
			open.pop_front();

			current_tile.getWalkableAdjacents(adjacents, &current_tile);

			for (int i = 0; i < adjacents.size(); i++)
			{
				adjacents[i].CalculateScore(destination);
				FieldNode* flow_field_node = flow_field->getNodeAt(adjacents[i].position);

				if (adjacents[i].score < flow_field_node->score)
				{
					*flow_field_node = adjacents[i];
					flow_field_node->parent = flow_field->getNodeAt(current_tile.position);
					open.push_back(*flow_field_node);

				}
			}
			adjacents.clear();
		}

		flow_field->stage = COMPLETED;
		break;
	default: 
		break;
	}

	return true;
}
