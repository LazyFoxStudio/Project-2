#include "p2Log.h"
#include "j1App.h"
#include "j1PathFinding.h"

j1PathFinding::j1PathFinding() : j1Module(), map(nullptr),width(0), height(0)
{
	name = "pathfinding";
}

// Destructor
j1PathFinding::~j1PathFinding()
{
	RELEASE_ARRAY(map);
}

// Called before quitting
bool j1PathFinding::CleanUp()
{
	LOG("Freeing pathfinding library");

	last_path.clear();
	RELEASE_ARRAY(map);
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
	return (pos.x >= 0 && pos.x <= (int)width &&
			pos.y >= 0 && pos.y <= (int)height);
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

	for(int i = -1;  i <= 1; i++)
		for (int j = -1; j <= 1; j++)
		{
			if (i != 0 || j != 0)
			{
				cell.create(pos.x + i, pos.y + j);
				if (App->pathfinding->IsWalkable(cell))
					list_to_fill.list.push_back(PathNode(-1, -1, cell, parent));
			}
		}

	return list_to_fill.list.size();
}

bool j1PathFinding::GatherWalkableAdjacents(iPoint map_pos, int count, std::vector<iPoint>& adjacents, int max_distance)
{
	if (!max_distance) max_distance = MAX_ADJACENT_DIST;

	iPoint cell;
	int radius = 1;
	uint found = 0;

	while (radius < max_distance)
	{
		for (int i = -radius; i <= radius; i++)
			for (int j = -radius; j <= radius; j++)
				if (std::abs(i) == radius || j == std::abs(j))
				{
					cell.create(map_pos.x + i, map_pos.y + j);
					if (App->pathfinding->IsWalkable(cell))
					{
						adjacents.push_back(cell);
						found++;
						if (found == count) return true;
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
		if (destination.x == -1) return ret;
	}

	if (IsWalkable(origin))
	{
		PathList open;
		PathList closed;

		PathNode originNode(0, origin.DistanceNoSqrt(destination), origin, nullptr);

		open.list.push_back(originNode);

		while (open.list.size())
		{
			ret++;
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

			for(std::list<PathNode>::iterator current = neighbors.list.begin(); current != neighbors.list.end(); current++)
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
	}

	return ret;
}

