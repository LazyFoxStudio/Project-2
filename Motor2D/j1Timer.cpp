// ----------------------------------------------------
// j1Timer.cpp
// Fast timer with milisecons precision
// ----------------------------------------------------

#include "j1Timer.h"
#include "SDL\include\SDL_timer.h"

// ---------------------------------------------
j1Timer::j1Timer()
{
	Start();
}

// ---------------------------------------------
void j1Timer::Start()
{
	if (!isPaused)
		started_at = paused_at = SDL_GetTicks();
	else
		started_at = SDL_GetTicks() - paused_at;

	isPaused = false;
}

void j1Timer::Restart()
{
	isPaused = false;
	started_at = paused_at = SDL_GetTicks();
}

// ---------------------------------------------
uint32 j1Timer::Read() const
{
	if (!isPaused)
		return SDL_GetTicks() - started_at;
	else
		return paused_at;
}

// ---------------------------------------------
float j1Timer::ReadSec() const
{
	if (!isPaused)
		return float(SDL_GetTicks() - started_at) / 1000.0f;
	else
		return float(paused_at) / 1000.0f;
}

void j1Timer::PauseTimer()
{
	if (!isPaused)
	{
		paused_at = SDL_GetTicks() - started_at;
		isPaused = true;
	}
}