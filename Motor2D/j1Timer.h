#ifndef __j1TIMER_H__
#define __j1TIMER_H__

#include "p2Defs.h"

class j1Timer
{
public:

	// Constructor
	j1Timer();

	void Start();
	void Restart();
	uint32 Read() const;
	float ReadSec() const;
	void PauseTimer();

public:

	bool	isPaused = false;

private:

	uint32	started_at;
	uint32  paused_at;
};

#endif //__j1TIMER_H__