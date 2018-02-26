#include "p2Defs.h"
#include "p2Log.h"
#include "j1Audio.h"
#include "p2List.h"

#include "SDL/include/SDL.h"
#include "SDL_mixer\include\SDL_mixer.h"
#pragma comment( lib, "SDL_mixer/libx86/SDL2_mixer.lib" )

j1Audio::j1Audio() : j1Module()
{
	music = NULL;
	name.create("audio");
}

// Destructor
j1Audio::~j1Audio()
{}

// Called before render is available
bool j1Audio::Awake(pugi::xml_node& config)
{
	//Stores the path names to each folder
	music_folder = config.child("music").child_value("folder");
	sfx_folder = config.child("sfx").child_value("folder");
	musicVolumeModifier = config.child("music").child("musicVolumeModifier").attribute("value").as_float();
	sfxVolumeModifier = config.child("sfx").child("sfxVolumeModifier").attribute("value").as_float();

	LOG("Loading Audio Mixer");
	bool ret = true;
	SDL_Init(0);

	if(SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
	{
		LOG("SDL_INIT_AUDIO could not initialize! SDL_Error: %s\n", SDL_GetError());
		active = false;
		ret = true;
	}

	// load support for the OGG audio format
	int flags = MIX_INIT_OGG;
	int init = Mix_Init(flags);

	if((init & flags) != flags)
	{
		LOG("Could not initialize Mixer lib. Mix_Init: %s", Mix_GetError());
		active = false;
		ret = true;
	}

	//Initialize SDL_mixer
	if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		LOG("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
		active = false;
		ret = true;
	}

	return ret;
}

// Called before quitting
bool j1Audio::CleanUp()
{
	if(!active)
		return true;

	LOG("Freeing sound FX, closing Mixer and Audio subsystem");

	if(music != NULL)
	{
		Mix_FreeMusic(music);
	}

	p2List_item<Mix_Chunk*>* item;
	for(item = fx.start; item != NULL; item = item->next)
		Mix_FreeChunk(item->data);

	fx.clear();

	Mix_CloseAudio();
	Mix_Quit();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);

	return true;
}

bool j1Audio::Save(pugi::xml_node & config) const
{
	config.append_child("musicVolumeModifier").append_attribute("value") = musicVolumeModifier;
	config.append_child("sfxVolumeModifier").append_attribute("value") = sfxVolumeModifier;
	return true;
}

bool j1Audio::Load(pugi::xml_node & config)
{
	musicVolumeModifier = config.child("musicVolumeModifier").attribute("value").as_float();
	sfxVolumeModifier = config.child("sfxVolumeModifier").attribute("value").as_float();

	Mix_VolumeMusic(128 * musicVolumeModifier);

	return true;
}

// Play a music file
bool j1Audio::PlayMusic(const char* path, float fade_time)
{
	bool ret = true;

	if(!active)
		return false;

	if(music != NULL)
	{
		if(fade_time > 0.0f)
		{
			Mix_FadeOutMusic(int(fade_time * 1000.0f));
		}
		else
		{
			Mix_HaltMusic();
		}

		// this call blocks until fade out is done
		Mix_FreeMusic(music);
	}
	 
	//Temporal string to merge the path to the folder and the sound file
	p2SString tmp("%s%s", music_folder.GetString(), path);

	music = Mix_LoadMUS(tmp.GetString());

	if(music == NULL)
	{
		LOG("Cannot load music %s. Mix_GetError(): %s\n", path, Mix_GetError());
		ret = false;
	}
	else
	{
		if(fade_time > 0.0f)
		{
			if(Mix_FadeInMusic(music, -1, (int) (fade_time * 1000.0f)) < 0)
			{
				LOG("Cannot fade in music %s. Mix_GetError(): %s", path, Mix_GetError());
				ret = false;
			}
		}
		else
		{
			if(Mix_PlayMusic(music, -1) < 0)
			{
				LOG("Cannot play in music %s. Mix_GetError(): %s", path, Mix_GetError());
				ret = false;
			}
		}
	}

	LOG("Successfully playing %s", path);
	return ret;
}

// Load WAV
unsigned int j1Audio::LoadFx(const char* path)
{
	unsigned int ret = 0;

	if(!active)
		return 0;

	//Temporal string to merge the path to the folder and the sound file
	p2SString tmp("%s%s", sfx_folder.GetString(), path);

	Mix_Chunk* chunk = Mix_LoadWAV(tmp.GetString());

	if(chunk == NULL)
	{
		LOG("Cannot load wav %s. Mix_GetError(): %s", path, Mix_GetError());
	}
	else
	{
		fx.add(chunk);
		ret = fx.count();
	}

	return ret;
}

// Play WAV
bool j1Audio::PlayFx(unsigned int id, int repeat, uint volume)
{
	bool ret = false;

	if(!active)
		return false;

	if(id > 0 && id <= fx.count())
	{
		Mix_VolumeChunk(fx[id - 1], volume*sfxVolumeModifier);
		Mix_PlayChannel(-1, fx[id - 1], repeat);
	}

	return ret;
}
void j1Audio::ModifyMusicVolume(int value)
{
	//LOG("%i", value);
	
	musicVolumeModifier = (float)value/100;
	LOG("%i", musicVolumeModifier);
	if (musicVolumeModifier < 0)musicVolumeModifier = 0;
	if (musicVolumeModifier > 1)musicVolumeModifier = 1;

	Mix_VolumeMusic(128 * musicVolumeModifier);
}
void j1Audio::ModifySFXVolume(int value)
{
	value = value / 100;
	sfxVolumeModifier = (float)value / 100;
	if (sfxVolumeModifier < 0)sfxVolumeModifier = 0;
	if (sfxVolumeModifier > 1)sfxVolumeModifier = 1;
}
