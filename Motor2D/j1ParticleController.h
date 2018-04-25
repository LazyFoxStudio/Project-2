#ifndef __J1PARTICLE_CONTROLLER_H__
#define __J1PARTICLE_CONTROLLER_H__

#include "j1Module.h"
#include "p2Point.h"
#include "p2Animation.h"

#define MAX_ACTIVE_PARTICLES 500

struct SDL_Texture;

enum particleType
{
	NO_TYPE,
	ARROW,
};

struct Particle
{
	Animation anim;
	fPoint position;
	fPoint speed;
	Uint32 born = 0;
	Uint32 life = 0;
	particleType type;

	Particle();
	Particle(Particle& p);
	~Particle() {};
	bool Update();
};

class j1ParticleController : public j1Module
{
public:
	j1ParticleController();
	~j1ParticleController();
	bool Awake();
	bool Start();
	bool Update(float dt);
	bool CleanUp();

	void LoadParticlesFromXML();

	void AddParticle(Particle& particle, int x, int y, float speed_x = 0.0f, float speed_y = 0.0f, Uint32 delay = 0, bool using_camera = true, particleType type = NO_TYPE);
	//void DeleteParticle();

private:

	std::list<Particle*> particleTemplates;
	SDL_Texture* graphics = nullptr;
	Particle* active[MAX_ACTIVE_PARTICLES];
	uint last_particle = 0;

public:

};



#endif // __MODULEPARTICLES_H__