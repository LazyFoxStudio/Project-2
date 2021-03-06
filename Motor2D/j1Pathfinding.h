#ifndef __j1PATHFINDING_H__
#define __j1PATHFINDING_H__

#include "j1Module.h"
#include "p2Point.h"

#include <list>

#define DEFAULT_PATH_LENGTH 50
#define INVALID_WALK_CODE 255
#define MAX_ADJACENT_DIST 13
#define FLOWFIELD_MAX 65535

#define PF_MAX_FRAMETIME 1.0f

class Entity;

enum PathProcessStage { REQUESTED, PROCESSING, COMPLETED, FAILED};

struct FieldNode
{
	int score = FLOWFIELD_MAX;
	FieldNode* parent = nullptr;
	iPoint position = { 0,0 };
	int g = 0;
	int h = 0;

	int CalculateScore(iPoint goal);
	void getWalkableAdjacents(std::vector<FieldNode>& list_to_fill, FieldNode* parent);
};

struct FlowField
{
	FieldNode** field = nullptr;
	uint width = 0; uint height = 0;
	PathProcessStage stage = REQUESTED;

	FlowField(uint width, uint heigth, int init_to = FLOWFIELD_MAX);
	~FlowField();

	void ClearTo(int value = FLOWFIELD_MAX);
	void DebugDraw();
	FieldNode* getNodeAt(iPoint p);

};

struct PathProcessor
{
	iPoint destination = { 0,0 };
	iPoint origin = { 0,0 };
	FlowField* flow_field = nullptr;
	bool to_erase = false;

	std::list<FieldNode> open;

	PathProcessor(iPoint destination, iPoint origin);
	~PathProcessor() { RELEASE(flow_field); };

	bool ProcessFlowField(j1Timer& timer);

};


class j1PathFinding : public j1Module
{
public:

	j1PathFinding();
	~j1PathFinding() {};

	// Called before quitting
	bool PostUpdate();
	bool CleanUp();

	// Sets up the walkability map
	void SetMap(uint width, uint height, uchar* data);

	// Main function to request a path from A to B
	int CreatePath(const iPoint& origin, iPoint& destination);

	// To request all tiles involved in the last generated path
	const std::list<iPoint>* GetLastPath() const;

	// Utility: return true if pos is inside the map boundaries
	bool CheckBoundaries(const iPoint& pos) const;

	// Utility: returns true is the tile is walkable
	bool IsWalkable(const iPoint& pos) const;

	// Utility: return the walkability value of a tile
	uchar GetTileAt(const iPoint& pos) const;

	bool GatherWalkableAdjacents(iPoint map_pos, int count, std::vector<iPoint>& adjacents, int max_distance = 0);
	iPoint FirstWalkableAdjacent(iPoint map_pos, int max_distance = 0);
	iPoint FirstWalkableAdjacentSafeProof(iPoint map_pos, iPoint dest, int max_distance = 0);

	FlowField* RequestFlowField(iPoint destination, iPoint origin);

public:
	uchar * map = nullptr;
	std::list<PathProcessor*> path_pool;
private:

	// size of the map
	uint width=0;
	uint height=0;
	// all map walkability values [0..255]
	
	// we store the created path here
	std::list<iPoint> last_path;
	j1Timer timer;
};

// forward declaration
struct PathList;

// ---------------------------------------------------------------------
// Pathnode: Helper struct to represent a node in the path creation
// ---------------------------------------------------------------------
struct PathNode
{
	// Convenient constructors
	PathNode();
	PathNode(int g, int h, const iPoint& pos, PathNode* parent);
	PathNode(const PathNode& node);

	// Fills a list (PathList) of all valid adjacent pathnodes
	uint FindWalkableAdjacents(PathList& list_to_fill, PathNode* parent);
	// Calculates this tile score
	int Score() const;
	// Calculate the F for a specific destination tile
	int CalculateF(const iPoint& destination);
	bool operator==(const PathNode& other)
	{
		return(this->h == other.h && this->g == other.g && this->parent == other.parent && this->pos == other.pos);
	}
	// -----------
	int g=0;
	int h=0;
	iPoint pos={ 0,0 };
	PathNode* parent=nullptr; // needed to reconstruct the path in the end
};

// ---------------------------------------------------------------------
// Helper struct to include a list of path nodes
// ---------------------------------------------------------------------
struct PathList
{
	// Looks for a node in this list and returns it's list node or NULL
	std::list<PathNode>::iterator Find(const iPoint& point);

	// Returns the Pathnode with lowest score in this list or NULL if empty
	PathNode GetNodeLowestScore() const;

	// -----------
	// The list itself, note they are not pointers!
	std::list<PathNode> list;
};



#endif // __j1PATHFINDING_H__