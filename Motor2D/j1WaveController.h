#ifndef __J1WAVECONTROLLER_H__
#define __J1WAVECONTROLLER_H__
#include "j1Module.h"
#include "j1Timer.h"
#include <list>
#include "p2Point.h"
class Squad;

class j1WaveController :
	public j1Module
{
public:
	j1WaveController();
	~j1WaveController();

	bool Awake(pugi::xml_node&);
	bool Start();
	bool Update(float dt);
	bool PostUpdate();
	bool CleanUp();

	bool Save(pugi::xml_node&) const;
	bool Load(pugi::xml_node&);

	int CalculateWaveScore();

	void GenerateWave();
private:
public:

	int initial_wait = 0;
	int wait_between_waves = 0;
	int current_wave = 0;
	j1Timer wave_timer;
	std::list<Squad*> wave;
	fPoint spawn_1;
	fPoint spawn_2;
	fPoint spawn_3;
	fPoint spawn_4;
};
#endif
