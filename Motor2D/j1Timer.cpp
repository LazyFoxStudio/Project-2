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
		started_at = SDL_GetTicks();
	else
		started_at += Read() - paused_at;

	isPaused = false;
}

// ---------------------------------------------
uint32 j1Timer::Read() const
{
	if (!isPaused)
		return float(SDL_GetTicks() - started_at);
	else
		return float(paused_at - started_at);
}

// ---------------------------------------------
float j1Timer::ReadSec() const
{
	if (!isPaused)
		return float(SDL_GetTicks() - started_at) / 1000.0f;
	else
		return float(paused_at - started_at) / 1000.0f;
}

void j1Timer::PauseTimer()
{
	paused_at = Read();
	isPaused = true;
}