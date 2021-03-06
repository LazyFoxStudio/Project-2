#include <math.h>
#include "j1App.h"
#include "j1Textures.h"
#include "j1Window.h"
#include "j1Render.h"
#include "j1Input.h"
#include "j1ParticleController.h"


j1ParticleController::j1ParticleController()
{
	for (uint i = 0; i < MAX_ACTIVE_PARTICLES; ++i)
		active[i] = nullptr;
}

j1ParticleController::~j1ParticleController()
{}

// Load assets
bool j1ParticleController::Awake(pugi::xml_node& config)
{
	name = "particle";

	LoadParticlesFromXML();

	return true;
}

bool j1ParticleController::Start()
{
	graphics = App->tex->Load("Assets/Sprites/Projectiles and Particles.png");
	return true;
}

// Unload assets
bool j1ParticleController::CleanUp()
{
	App->tex->UnLoad(graphics);

	for (uint i = 0; i < MAX_ACTIVE_PARTICLES; ++i)
	{
		if (active[i] != nullptr)
		{
			delete active[i];
			active[i] = nullptr;
		}
	}
	for (std::list<Particle*>::reverse_iterator it = particleTemplates.rbegin(); it != particleTemplates.rend() && (*it) != nullptr; it++)
	{
		delete *it;
		*it = nullptr;
	}
	return true;
}


// Update: draw background
bool j1ParticleController::Update(float dt)
{
	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_DOWN)
		AddProgressiveParticle(PFLAMETHROWER, { 100,100 }, { 1000,1000 });

	for (uint i = 0; i < MAX_ACTIVE_PARTICLES; ++i)
	{
		Particle* p = active[i];

		if (p == nullptr)
			continue;

		if (p->Update(dt) == false)
		{
			delete p;
			active[i] = nullptr;
		}
		else if (p->currentLife.Read() > 0)
		{
			if (App->render->CullingCam(p->position) && p->active)
			{
				SDL_Rect frame = p->anim.GetCurrentFrame(dt);
				if(p->speed.x >= 0 || p->parabollic == true)
					App->render->Blit(graphics, p->position.x - (frame.w*p->scale) / 2, p->position.y - (frame.h*p->scale) / 2, &frame, true, false, p->scale, SDL_FLIP_NONE, 1, RADTODEG * p->angle);
				else
					App->render->Blit(graphics, p->position.x - (frame.w*p->scale) / 2, p->position.y - (frame.h*p->scale) / 2, &frame, true, false, p->scale, SDL_FLIP_NONE, 1, 180 - RADTODEG * p->angle);

			}
		}


	}

	return true;
}


void j1ParticleController::LoadParticlesFromXML()
{
	pugi::xml_document Particledoc;
	pugi::xml_node file;

	file = App->LoadFile(Particledoc, "Particle_Templates.xml");

	Particle* tmp; 
	int type = 1;

	for (pugi::xml_node part = file.child("particle").child("template"); part; part = part.next_sibling("template"))
	{
		tmp = new Particle();

		tmp->type = (particleType)type++;

		tmp->width = part.child("width").attribute("value").as_int(0);
		tmp->height = part.child("height").attribute("value").as_int(0);
		tmp->parabollic = part.child("parabollic").attribute("value").as_bool(false);

		pugi::xml_node childNode = part.child("anim");

		tmp->anim.speed = childNode.child("speed").attribute("value").as_float(0);
		tmp->anim.loop = childNode.child("loop").attribute("value").as_bool(true);
		
		for (pugi::xml_node pb = childNode.child("pushback"); pb; pb = pb.next_sibling("pushback"))
			tmp->anim.PushBack({ pb.attribute("x").as_int(0), pb.attribute("y").as_int(0), pb.attribute("w").as_int(0), pb.attribute("h").as_int(0) });

		particleTemplates.push_back(tmp);
	}

}

Particle* j1ParticleController::FindParticleType(particleType type)
{
	Particle tmp;

	for (std::list<Particle*>::iterator it = particleTemplates.begin(); it != particleTemplates.end(); it++)
		if ((*it)->type == type) return *it;

}

void j1ParticleController::AdjustDirection(Particle* p, fPoint objective, float speed)
{
	fPoint vec = { objective.x - p->position.x - (p->width/2), objective.y - p->position.y - (p->height / 2) };

	p->angle = asinf(vec.y/vec.GetModule());
	float sinus = sinf(p->angle);
	float cosinus = cosf(p->angle);
	p->speed.y = speed * sinus;
	p->speed.x = speed * cosinus;

	if (vec.x < 0)
		p->speed.x *= -1;

	p->life = (vec.GetModule() / speed)*1000;
}



void j1ParticleController::AddParticle(particleType type, fPoint position, bool using_center, float scale)
{
	for (uint i = 0; i < MAX_ACTIVE_PARTICLES; ++i)
	{
		if (active[i] == nullptr)
		{
			Particle* p = new Particle(*FindParticleType(type));

			if (using_center)
			{
				p->position.x = position.x - App->render->camera.x;
				p->position.y = position.y - App->render->camera.y;
			}
			else
			{
				p->position.x = position.x;
				p->position.y = position.y;
			}
			p->speed.SetToZero();

			p->currentLife.Start();
			p->active = true;
			p->delay = 0;
			p->scale = scale;
			p->angle = 0;

			active[i] = p;
			break;
		}
	}
}

void j1ParticleController::AddProjectile(particleType type, fPoint position, fPoint objective, float speed, bool using_camera, float scale)
{

	for (uint i = 0; i < MAX_ACTIVE_PARTICLES; ++i)
	{
		if (active[i] == nullptr)
		{
			Particle* p = new Particle(*FindParticleType(type));
			p->currentLife.Start();

			p->position.x = position.x;
			p->position.y = position.y;

			if (speed != 0)
					AdjustDirection(p, objective, speed);
			else
			break;

			p->active = true;
			p->delay = 0;
			p->scale = scale;

			active[i] = p;
			break;
		}
	}
}

void j1ParticleController::AddProgressiveParticle(particleType type, fPoint position, fPoint objective, float spread, float numofseparations, bool using_camera, float negativespread)
{
	fPoint vec = { objective.x - position.x, objective.y - position.y };
	vec.x = vec.x / numofseparations;
	vec.y = vec.y / numofseparations;

	for (int j = 0; j != numofseparations+1; j++)
	{
		for (uint i = 0; i < MAX_ACTIVE_PARTICLES; ++i)
		{
			if (active[i] == nullptr)
			{
				Particle* p = new Particle(*FindParticleType(type));
				p->currentLife.Start();

				p->position.x = position.x + (float)(vec.x*j);
				p->position.y = position.y + (float)(vec.y*j);

				p->delay = 1 + j*(500 / numofseparations);

				float tmp = (float)(1 / numofseparations);
				p->scale = negativespread + (spread - negativespread) * ((float)(tmp * j));
				p->angle = 0;

				active[i] = p;
				break;
			}
		}
	}
}

Particle::Particle()
{
	position.SetToZero();
	speed.SetToZero();
}

Particle::Particle(Particle& p) :
	anim(p.anim), position(p.position), life(p.life), delay(p.delay), scale(p.scale),
	type(p.type), width(p.width), height(p.height), parabollic(p.parabollic)
{}


bool Particle::Update(float dt)
{
	bool ret = true;
	int aux = (currentLife.Read());
	if (delay > 0)
	{
		if (aux > delay)
		{
			if (!active)
			{
				active = true;
				anim.Reset();
			}
		}
		else
		{
			return ret;
		}
	}

	if (life > 0)
	{
		if (aux > (int)life)
			ret = false;
	}
	else
	{
		if (anim.Finished())
			ret = false;
	}
	
	if (parabollic)
	{
		float z = (((life * 0.5f - aux) / (life * 0.5f)) * (life / 100));
		angle = (fPoint(speed.x / speed.GetModule(), speed.y / speed.GetModule() - (z / 5))).GetAngle();
		position.y -= z;
	}
	
	position.x += speed.x*dt;
	position.y += speed.y*dt;
	
	return ret;
}

