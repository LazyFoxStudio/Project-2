// ----------------------------------------------------
// j1Timer.cpp
// Fast timer with milisecons precision
// ----------------------------------------------------

#include "j1Timer.h"
#include "SDL\include\SDL_timer.h"
#include "j1App.h"

// ---------------------------------------------
j1Timer::j1Timer()
{
	started_at = paused_at = SDL_GetTicks();
}

// ---------------------------------------------
void j1Timer::Start()
{
	if (!isPaused)
		started_at = paused_at = App->gameTime.ReadMs();
	else
		started_at = App->gameTime.ReadMs() - paused_at;

	isPaused = false;
}

void j1Timer::Restart()
{
	isPaused = false;
	started_at = paused_at = App->gameTime.ReadMs();
}

// ---------------------------------------------
uint32 j1Timer::Read() const
{
	if (!isPaused)
		return App->gameTime.ReadMs() - started_at;
	else
		return paused_at;
}

// ---------------------------------------------
float j1Timer::ReadSec() const
{
	if (!isPaused)
		return float(App->gameTime.ReadMs() - started_at) / 1000.0f;
	else
		return float(paused_at) / 1000.0f;
}

void j1Timer::PauseTimer()
{
	if (!isPaused)
	{
		paused_at = App->gameTime.ReadMs() - started_at;
		isPaused = true;
	}
}