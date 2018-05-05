#ifndef __j1PERFTIMER_H__
#define __j1PERFTIMER_H__

#include "p2Defs.h"

class j1PerfTimer
{
public:

	// Constructor
	j1PerfTimer();

	void Start();
	double ReadMs() const;
	float ReadSec() const;
	uint64 ReadTicks() const;
	void PauseTimer();

public:

	bool isPaused = false;

private:
	uint64	started_at;
	uint64  paused_at;
	static uint64 frequency;
};

#endif //__j1PERFTIMER_H__