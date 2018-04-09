#include "j1WaveController.h"



j1WaveController::j1WaveController()
{
	name = { "wavecontroller" };
}


j1WaveController::~j1WaveController()
{
}

bool j1WaveController::Awake(pugi::xml_node &config)
{
	initial_wait = config.child("initialWait").attribute("value").as_int(0);
	wait_between_waves = config.child("waitBetweenWaves").attribute("value").as_int(0);

	return true;
}

bool j1WaveController::Start()
{
	wave_timer.Start();
	return true;
}

bool j1WaveController::Update(float dt)
{
	if (current_wave == 0 && wave_timer.ReadSec() > initial_wait)
	{
		current_wave += 1;
		wave_timer.Start();
	}

	else if (current_wave != 0 && wave_timer.ReadSec() > wait_between_waves)
	{
		current_wave += 1;
		wave_timer.Start();
	}
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

int j1WaveController::CalculateWaveScore()
{
	int ret = 0;
	ret = current_wave * 10;

	return ret;
}
