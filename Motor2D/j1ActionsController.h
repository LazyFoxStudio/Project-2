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
	CREATE_ARCHER,
	CREATE_KNIGHT,
	CREATE_GRYPHON,
	CREATE_BALLISTA,
	CREATE_FLYING_MACHINE,
	USE_ABILITY1,
	USE_ABILITY2,
	USE_ABILITY3,
	REPAIR_BUILDING,
	DEMOLISH_BUILDING,
	TOGGLE_NEXTWAVE,
	BUILD_MINE,
	BUILD_TURRET,
	BUILD_GNOME_HUT,
	BUILD_CHURCH,
	BUILD_BLACKSMITH,
	RESEARCH_MELEE_ATTACK,
	RESEARCH_MELEE_DEFENSE,
	RESEARCH_RANGED_ATTACK,
	RESEARCH_RANGED_DEFENSE,
	RESEARCH_FLYING_ATTACK,
	RESEARCH_FLYGIN_DEFENSE,

	NEW_GAME,
	SETTINGS,
	BACK_MENU,
	CREDITS,
	EXIT,
	CROSS_MENU,
	CHANGE_HOTKEYS,
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
	actionType action_type=NO_ACTION;
	fPoint newSquadPos = { 0.0f,0.0f };

};

#endif // !__J1ACTIONSCONTROLLER_H__