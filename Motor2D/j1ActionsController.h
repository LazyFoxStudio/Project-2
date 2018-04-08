#ifndef __J1ACTIONSCONTROLLER_H__
#define __J1ACTIONSCONTROLLER_H__

#include "j1Module.h"

enum actionType
{
	NO_ACTION = 0,

	MOVE,
	BUILD_BARRACKS,
	BUILD_LUMBER_MILL,
	BUILD_FARM,
	UNASSIGN_WORKER,
	ASSIGN_WORKER
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
	bool action_assigned = false;
	actionType action_type;

};

#endif // !__J1ACTIONSCONTROLLER_H__