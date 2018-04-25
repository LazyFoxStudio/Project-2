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
bool j1ParticleController::Awake()
{
	
	return true;
}

bool j1ParticleController::Start()
{
	graphics = App->tex->Load("assets/images/particles.png");
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

	return true;
}

void j1ParticleController::LoadParticlesFromXML()
{
	/*pugi::xml_document Particledoc;
	pugi::xml_node file;

	file = App->LoadFile(Particledoc, "Audio_Paths.xml");

	for (pugi::xml_node part = file.child("SFX").child("path"); part; part = part.next_sibling("path"))
		LoadFx(SFX.attribute("sfx").as_string());*/

}

// Update: draw background
bool j1ParticleController::Update(float dt)
{
	for (uint i = 0; i < MAX_ACTIVE_PARTICLES; ++i)
	{
		Particle* p = active[i];

		if (p == nullptr)
			continue;

		if (p->Update() == false)
		{

			delete p;
			active[i] = nullptr;
		}
		else if (SDL_GetTicks() >= p->born)
		{
			App->render->Blit(graphics, p->position.x, p->position.y, &(p->anim.GetCurrentFrame(dt)), false);
		}


	}

	return true;
}

void j1ParticleController::AddParticle(Particle& particle, int x, int y, float speed_x, float speed_y, Uint32 delay,  bool using_camera, particleType type)
{
	for (uint i = 0; i < MAX_ACTIVE_PARTICLES; ++i)
	{
		if (active[i] == nullptr)
		{
			Particle* p = new Particle(particle);
			p->born = SDL_GetTicks() + delay;
			
			if(speed_x != 0)
			p->speed.x = speed_x;
			if(speed_y != 0)
			p->speed.y = speed_y;
			

			p->position.x = x;
			p->position.y = y;
		
			p->type = type;
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
	anim(p.anim), position(p.position), speed(p.speed),
	born(p.born), life(p.life), type(p.type)
{}


bool Particle::Update()
{
	bool ret = true;
	int aux = (SDL_GetTicks() - born);

	if (life > 0)
	{
		if (aux > (int)life)
			ret = false;
	}
	else
		if (anim.Finished())
			ret = false;

	
	
	position.x += speed.x;
	position.y += speed.y;
	

	if (position.x > App->win->width || position.x < 0 || position.y >  App->win->height || position.y < 0)
		life = 0;

	return ret;
}

