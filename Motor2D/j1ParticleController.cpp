#include <math.h>
#include "j1App.h"
#include "j1Textures.h"
#include "j1Window.h"
#include "j1Render.h"
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
			App->render->Blit(graphics, p->position.x, p->position.y, &(p->anim.GetCurrentFrame(dt)), true, false, 1, SDL_FLIP_NONE, 1, GetAngleInDegrees(p));
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

		tmp->type = GetTypeFromInt(type++);

		tmp->width = part.child("width").attribute("value").as_int(0);
		tmp->height = part.child("height").attribute("value").as_int(0);

		pugi::xml_node childNode = part.child("anim");

		tmp->anim.speed = childNode.child("speed").attribute("value").as_float(0);
		tmp->anim.loop = childNode.child("loop").attribute("value").as_bool(true);
		
		for (pugi::xml_node pb = childNode.child("pushback"); pb; pb = pb.next_sibling("pushback"))
			tmp->anim.PushBack({ pb.attribute("x").as_int(0), pb.attribute("y").as_int(0), pb.attribute("w").as_int(0), pb.attribute("h").as_int(0) });

		particleTemplates.push_back(tmp);
	}

}

particleType j1ParticleController::GetTypeFromInt(int posOnEnum)
{
	switch (posOnEnum)
	{
	case 9:
		return particleType::PJUGGERNAUT;
	case 8:
		return particleType::PCATAPULT;
	case 7:
		return particleType::PDRAGON;
	case 6:
		return particleType::PDEATHKNIGHT;
	case 5:
		return particleType::PFLYINGMACHINE;
	case 4:
		return particleType::PBALLISTA;
	case 3:
		return particleType::PYAHMAM_AA;
	case 2:
		return particleType::PTOMAHAWK;
	case 1:
		return particleType::PARROW;
	default:
		return particleType::PNO_TYPE;
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
	fPoint vec = { objective.x - p->position.x + (p->width/2), objective.y - p->position.y + (p->height / 2) };

	p->angle = asinf(vec.y/vec.GetModule());
	float sinus = sinf(p->angle);
	float cosinus = cosf(p->angle);
	p->speed.y = speed * sinus;
	p->speed.x = speed * cosinus;

	if (vec.x < 0)
		p->speed.x *= -1;


	p->life = (vec.GetModule() / speed)*1000;
}

double j1ParticleController::GetAngleInDegrees(Particle * p)
{
	double angle = p->angle * 180 / M_PI;

	if (p->speed.x <= 0)
		angle = 180 - angle;
	
	return angle;
}


void j1ParticleController::AddParticle(particleType type, fPoint position, bool using_camera)
{
	for (uint i = 0; i < MAX_ACTIVE_PARTICLES; ++i)
	{
		if (active[i] == nullptr)
		{
			Particle* p = new Particle(*FindParticleType(type));
			p->currentLife.Start();


			p->position.x = position.x;
			p->position.y = position.y;
			p->speed.SetToZero();


			active[i] = p;
			break;
		}
	}
}

void j1ParticleController::AddProjectile(particleType type, fPoint position, fPoint objective, float speed, bool using_camera)
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

			active[i] = p;
			break;
		}
	}
}

// Funtion that takes care of its collision
//void j1ParticleController::OnCollision(Collider* c1, Collider* c2)
//{
//	// Colliders that are deleted on contact are deleted here
//	if (c1->type == COLLIDER_PLAYER_SHOT || c1->type == COLLIDER_PLAYER2_SHOT || c1->type == COLLIDER_ENEMY_SHOT || c2->type == COLLIDER_WALL)
//	{
//		for (uint i = 0; i < MAX_ACTIVE_PARTICLES; ++i)
//		{
//			if (active[i] != nullptr && active[i]->collider == c1)
//			{
//				delete active[i];
//				active[i] = nullptr;
//				break;
//			}
//		}
//	}
//}


// -------------------------------------------------------------
// -------------------------------------------------------------

Particle::Particle()
{
	position.SetToZero();
	speed.SetToZero();
}

Particle::Particle(Particle& p) :
	anim(p.anim), position(p.position),
	life(p.life), type(p.type)
{}


bool Particle::Update(float dt)
{
	bool ret = true;
	int aux = (currentLife.Read());

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
	
	
	position.x += speed.x*dt;
	position.y += speed.y*dt;
	

	if (position.x > (App->win->width + -App->render->camera.x) || position.x < -App->win->width || position.y > (App->win->height + -App->render->camera.y) || position.y < -App->win->height)
		life = 1;

	return ret;
}

