#ifndef __J1ACTIONSCONTROLLER_H__
#define __J1ACTIONSCONTROLLER_H__

#include "j1Module.h"
#include "p2Point.h"

enum actionType
{
	NO_ACTION = 0,

	MOVE,
	BUILD_BARRACKS,
	BUILD_LUMBER_MILL,
	BUILD_FARM,
	UNASSIGN_WORKER,
	ASSIGN_WORKER,
	CREATE_FOOTMAN,
	CREATE_ARCHER
};

class j1ActionsController : public j1Module
{
public:

	j1ActionsController()
	{
		name = "actionscontroller";
	}

	~j1ActionsController()
	{}

	bool Update(float dt);

	void activateAction(actionType type);

public:

	bool doingAction = false;
	bool doingAction_lastFrame = false;
	bool action_assigned = false;
	actionType action_type;
	fPoint newSquadPos = { 0.0f,0.0f };

};

#endif // !__J1ACTIONSCONTROLLER_H__