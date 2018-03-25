#include "Ally.h"
#include "Command.h"

Ally::~Ally()
{

}

bool Ally::Update(float dt)
{
	if (!commands.empty())
	{
		commands.front()->Execute(dt);
		if (commands.front()->state == FINISHED) commands.pop_front();
	}

	return true;
}

void Ally::attackEntity(Entity* target)
{

}

Entity* Ally::findTarget()
{
	return nullptr;
}
