#include "j1WaveController.h"



j1WaveController::j1WaveController()
{
	name = { "wavecontroller" };
}


j1WaveController::~j1WaveController()
{
}

bool j1WaveController::Awake(pugi::xml_node &)
{
	return true;
}

bool j1WaveController::Start()
{
	return true;
}

bool j1WaveController::Update(float dt)
{
	return true;
}

bool j1WaveController::PostUpdate()
{
	return true;
}

bool j1WaveController::CleanUp()
{
	return true;
}

bool j1WaveController::Save(pugi::xml_node &) const
{
	return true;
}

bool j1WaveController::Load(pugi::xml_node &)
{
	return true;
}
