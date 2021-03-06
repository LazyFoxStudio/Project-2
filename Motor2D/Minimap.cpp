#include "Minimap.h"
#include "j1App.h"
#include "j1Audio.h"
#include "j1Render.h"
#include "j1Input.h"
#include "p2Log.h"
#include "j1Gui.h"

Minimap::Minimap(const char* base_texture_path, int _window_position_x, int _window_position_y, int _width, int _height, int tex_width, int tex_height)
{
	if (tex_width == -1 && tex_height == -1)
	{
		base_image = IMG_Load(base_texture_path);
	}
	else
	{
		base_image = SDL_CreateRGBSurface(0, tex_width, tex_height, 32, 0, 0, 0, 0);
		SDL_FillRect(base_image, NULL, SDL_MapRGB(base_image->format, 255, 0, 0));
	}

	map_width = _width;
	map_height = _height;
	width = base_image->w;
	height = base_image->h;

	window_position_x = _window_position_x;
	window_position_y = _window_position_y;

	ratio_x = (float)width	/ (float)map_width;
	ratio_y = (float)height	/ (float)map_height;
}

Minimap::~Minimap()
{
	if (base_image != nullptr)
	{
		SDL_FreeSurface(base_image);
		base_image = nullptr;
	}

	for (std::list<alert_def>::iterator it = alert_images.begin(); it != alert_images.end(); it++)
	{
		SDL_FreeSurface(it->img);
	}

	alert_images.clear();

	for (std::list<alert*>::iterator it = alert_queue.begin(); it != alert_queue.end(); it++)
	{
		RELEASE(*it);
	
	}
	alert_queue.clear();

	point_queue.clear();
	sprite_queue.clear();
}

SDL_Texture * Minimap::createTexture()
{
	//this is the texture we will be blitting, a copy of the actual base, so that way we are not modifying it
	SDL_Surface* manipulable = new SDL_Surface();
	manipulable = SDL_ConvertSurface(base_image, base_image->format, SDL_SWSURFACE);

	//we first blit the sprites that the player asked for to be in the map for this frame
	for (std::list<sprite>::iterator it = sprite_queue.begin(); it != sprite_queue.end(); it++)
	{
		SDL_Rect r = { it->section.x*ratio_x, it->section.y *ratio_y, it->section.w, it->section.h };
		SDL_BlitSurface(it->sprite_surface, NULL, manipulable, &r);
	}
	sprite_queue.clear();

	//now we begin with the points
	for (std::list<point>::iterator it = point_queue.begin(); it != point_queue.end(); it++)
	{
		SDL_Rect representation;
		// we multiply the rectangle by the ratio so that it gets reduced to minimap size
		representation.x = ratio_x * it->rect.x;
		representation.y = ratio_y * it->rect.y;
		representation.w = ratio_x * it->rect.w;
		representation.h = ratio_y * it->rect.h;

		//now we make it fit inside the modifiable texture
		SDL_FillRect(manipulable, &representation, SDL_MapRGB(manipulable->format, it->color.r, it->color.g, it->color.b));
	}

	point_queue.clear();

	for (std::list<alert*>::iterator it = alert_queue.begin(); it != alert_queue.end(); it++)
	{
		//now we make it fit inside the modifiable texture
		SDL_Surface* s = nullptr;
		SDL_Rect r = { (*it)->x*ratio_x, (*it)->y *ratio_y, 1, 1 };//PROVISIONAL

		for (std::list<alert_def>::iterator it_images = alert_images.begin(); it_images != alert_images.end(); it_images++)
		{
			if ((*it)->type == it_images->type)
			{
				s = it_images->img;
				r.w = s->w;//PROVISIONAL
				r.h = s->h;

				if (!(*it)->sound_played)
				{
					if ((*it)->type == DANGER)
					{
						App->audio->PlayFx(SFX_MAP_ATTACK);
					}
					else
					{
						App->audio->PlayFx(SFX_MAP_ASSIST);
					}
					(*it)->sound_played = true;
				}
				break;
			}
		}
		if (s)
		{
			SDL_BlitSurface(s, NULL, manipulable, &r);
		}
		if (SDL_GetTicks() >(*it)->time_in_seconds * 1000 + (*it)->time_started_at)
		{
			(*it)->to_delete = true;
		}
	}
	for (std::list<alert*>::iterator it = alert_queue.begin(); it != alert_queue.end(); it++)
	{
		if ((*it)->to_delete)
		{
			RELEASE(*it)
				alert_queue.remove(*it);
			//it--;
		}
	}
	//now we will blit the viewport representation on the minimap
	SDL_Rect up = { -App->render->camera.x * ratio_x,-App->render->camera.y * ratio_y ,App->render->camera.w * ratio_x, 1 };
	SDL_FillRect(manipulable, &up, SDL_MapRGB(manipulable->format, 255, 255, 255));

	SDL_Rect down = { -App->render->camera.x * ratio_x,-(App->render->camera.y - App->render->camera.h) * ratio_y - 1 ,App->render->camera.w * ratio_x, 1 };
	SDL_FillRect(manipulable, &down, SDL_MapRGB(manipulable->format, 255, 255, 255));

	SDL_Rect left = { -App->render->camera.x * ratio_x,-App->render->camera.y * ratio_y ,1 , App->render->camera.h * ratio_y };
	SDL_FillRect(manipulable, &left, SDL_MapRGB(manipulable->format, 255, 255, 255));

	SDL_Rect right = { -(App->render->camera.x - App->render->camera.w) * ratio_x - 1 , -App->render->camera.y * ratio_y ,1, App->render->camera.h* ratio_y };
	SDL_FillRect(manipulable, &right, SDL_MapRGB(manipulable->format, 255, 255, 255));

	//we create the texture
	SDL_Texture* ret = SDL_CreateTextureFromSurface(App->render->renderer, manipulable);
	SDL_FreeSurface(manipulable);
	manipulable = nullptr;
	
	return ret;
}

void Minimap::DrawMinimap()
{
	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT && !App->isPaused())
		moveCamera();

	//We get the texture
	SDL_Texture* texture_to_blit = createTexture();
	
	// we blit it
	App->render->Blit(texture_to_blit,window_position_x, window_position_y, nullptr, false, true);

	//Free the texture we just created
	SDL_DestroyTexture(texture_to_blit);	
}

void Minimap::moveCamera()
{
	int camx, camy;
	Mouse_to_map(camx, camy);

	if (camx != -1 && camy != -1)
	{
		App->render->camera.y = -camy + App->render->camera.h / 2;
		App->render->camera.x = -camx + App->render->camera.w / 2;

		if (camx - App->render->camera.w / 2 > 4096 - App->render->camera.w) {
			LOG("TOP RIGHT");
			App->render->camera.x = -4096 + App->render->camera.w;
		}
		else if (camx - App->render->camera.w / 2 < 0) {
			LOG("TOP LEFT");
			App->render->camera.x = 0;
		}

		if (camy - App->render->camera.h / 2 > 4096 - App->render->camera.h) {
			LOG("TOP DOWN");
			App->render->camera.y = -4096 + App->render->camera.h;
		}
		else if (camy - App->render->camera.h / 2 < 0) {
			LOG("TOP UP");
			App->render->camera.y = 0;
		}
	}
}

void Minimap::Addpoint(SDL_Rect rect, Color color) 
{
	point p;
	p.rect = rect;
	p.color = { (Uint8)color.r, (Uint8)color.g, (Uint8)color.b, (Uint8)color.a };

	point_queue.push_back(p);
}

void Minimap::Draw_Sprite(SDL_Surface* img, int pos_x, int pos_y)
{
	sprite _sprite;

	_sprite.sprite_surface = img;
	_sprite.section.x = pos_x;
	_sprite.section.y = pos_y;
	_sprite.section.w = img->w;
	_sprite.section.h = img->h;

	sprite_queue.push_back(_sprite);
}

void Minimap::AddAlert(int x, int y, int time_in_seconds, alert_type type)
{
	alert* ret = new alert();

	ret->x = x;
	ret->y = y;
	ret->time_in_seconds = time_in_seconds;
	ret->type = type;

	alert_queue.push_back(ret);
}

void Minimap::AddAlertDef(const char* path, alert_type type)
{
	alert_def def;

	def.img = IMG_Load(path);
	def.type = type;

	alert_images.push_back(def);
}

void Minimap::Mouse_to_map(int& map_x, int& map_y)// returns -1 in the variables if unsuccesfull
{
	int mouse_x, mouse_y;
	App->input->GetMousePosition(mouse_x, mouse_y);

	SDL_Rect section = { window_position_x, window_position_y, width, height };
	section.x *= App->gui->w_stretch;
	section.y *= App->gui->h_stretch;
	section.w *= App->gui->w_stretch;
	section.h *= App->gui->h_stretch;
	if (	mouse_x > section.x && mouse_x < section.x + section.w
		&&	mouse_y > section.y && mouse_y < section.y + section.h)
	{
		map_x = (mouse_x - section.x)/ (ratio_x*App->gui->w_stretch);
		map_y = (mouse_y - section.y) / (ratio_y*App->gui->h_stretch);
	}
	else
	{
		map_x = -1;
		map_y = -1;
	}
}