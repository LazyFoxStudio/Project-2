#ifndef __J1WAVECONTROLLER_H__
#define __J1WAVECONTROLLER_H__
#include "j1Module.h"
#include "j1Timer.h"
#include "Unit.h"
#include <list>
#include "p2Point.h"

class Squad;
class FlowField;

struct NextWave
{
	NextWave(unitType type, fPoint spawn) :type(type), spawn(spawn)
	{};

	unitType	type;
	fPoint		spawn;
};

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

	void Generate_Next_Wave();
	void Generate_Wave();

	void updateFlowField();
public:

	FlowField*			flow_field = nullptr;
	FlowField*			flow_field_aux = nullptr;    // for swapping

	int					initial_wait = 0;
	int					wait_between_waves = 0;
	int					current_wave = 0;
	
	j1Timer				wave_timer;
	
	std::list<NextWave*> next_wave;

	iPoint TownHall_pos = { 0,0 };
	
	std::vector<fPoint> spawns;
};	
#endif
