#ifndef __J1PARTICLE_CONTROLLER_H__
#define __J1PARTICLE_CONTROLLER_H__

#include "j1Module.h"
#include "p2Point.h"
#include "p2Animation.h"
#include "j1Timer.h"

#define MAX_ACTIVE_PARTICLES 500

struct SDL_Texture;

enum particleType
{
	PNO_TYPE,
	PARROW,
	PTOMAHAWK,
	PYAHMAM_AA,
	PBALLISTA,
	PFLYINGMACHINE,
	PDEATHKNIGHT,
	PDRAGON,
	PCATAPULT,
	PJUGGERNAUT

};

struct Particle
{
	Animation anim;
	fPoint position;
	fPoint speed = { 0,0 };
	float angle;
	j1Timer currentLife;
	Uint32 life = 0;
	particleType type;
	int width = 0;
	int height = 0;

	Particle();
	Particle(Particle& p);
	~Particle() {};
	bool Update(float dt);
};

class j1ParticleController : public j1Module
{
public:
	j1ParticleController();
	~j1ParticleController();
	bool Awake(pugi::xml_node&);
	bool Start();
	bool Update(float dt);
	bool CleanUp();

	void LoadParticlesFromXML();
	particleType GetTypeFromInt(int posOnEnum);
	Particle* FindParticleType(particleType type);
	void AdjustDirection(Particle* p, fPoint objective, float speed);
	
	double GetAngleInDegrees(Particle* p);

	void AddParticle(particleType type = PNO_TYPE, fPoint position = { 0,0 }, bool using_camera = true);
	void AddProjectile(particleType type = PNO_TYPE, fPoint position = { 0,0 }, fPoint objective = { 0,0 },float speed = 0, bool using_camera = true);
	//void DeleteParticle();

private:

	std::list<Particle*> particleTemplates;
	SDL_Texture* graphics = nullptr;
	Particle* active[MAX_ACTIVE_PARTICLES];
	uint last_particle = 0;

public:

};



#endif // __MODULEPARTICLES_H__