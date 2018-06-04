#include "p2Defs.h"
#include "p2Log.h"
#include "j1Audio.h"
#include "j1App.h"

#include "SDL/include/SDL.h"
#include "SDL_mixer\include\SDL_mixer.h"
#pragma comment( lib, "SDL_mixer/libx86/SDL2_mixer.lib" )


j1Audio::j1Audio() : j1Module() { name = "audio"; pausable = false; }

// Destructor
j1Audio::~j1Audio() {}

// Called before render is available
bool j1Audio::Awake(pugi::xml_node& config)
{
	musicVolumeModifier = DEFAULT_MUSIC_MODIFIER;
	sfxVolumeModifier = DEFAULT_SFX_MODIFIER;

	LOG("Loading Audio Mixer");
	SDL_Init(0);

	if(SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
	{
		LOG("SDL_INIT_AUDIO could not initialize! SDL_Error: %s\n", SDL_GetError());
		return (active = false);
	}

	// load support for the OGG audio format
	int flags = MIX_INIT_OGG;
	int init = Mix_Init(flags);

	if((init & flags) != flags)
	{
		LOG("Could not initialize Mixer lib. Mix_Init: %s", Mix_GetError());
		return (active = false);
	}

	//Initialize SDL_mixer
	if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		LOG("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
		return (active = false);
	}

	LoadFXFromXML();
	return true;
}

// Called before quitting
bool j1Audio::CleanUp()
{
	if (active)
	{
		LOG("Freeing sound FX, closing Mixer and Audio subsystem");

		for (int i = 0; i < music.size(); i++) Mix_FreeMusic(music[i]);
		music.clear();

		for (int i = 0; i < fx.size(); i++)   Mix_FreeChunk(fx[i]);
		fx.clear();

		Mix_CloseAudio();
		Mix_Quit();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
	}
	return true;
}

bool j1Audio::PostUpdate()
{
	if (blackListCooldown.Read() > 300)
	{
		blackList.clear();
		blackListCooldown.Restart();
	}
	return true;
}

bool j1Audio::Save(pugi::xml_node & config) const
{
	//config.append_child("musicVolumeModifier").append_attribute("value") = musicVolumeModifier;
	//config.append_child("sfxVolumeModifier").append_attribute("value") = sfxVolumeModifier;
	return true;
}

bool j1Audio::Load(pugi::xml_node & config)
{
	//musicVolumeModifier = config.child("musicVolumeModifier").attribute("value").as_float();
	//sfxVolumeModifier = config.child("sfxVolumeModifier").attribute("value").as_float();

	//Mix_VolumeMusic(128 * musicVolumeModifier);

	return true;
}

// Play a music file

bool j1Audio::PlayMusic(uint id, uint fade_time, int loops)
{
	bool ret = false;

	if (active && id < music.size()) {

		if (current_track)
		{
			if (fade_time)	Mix_FadeOutMusic(int(fade_time * 1000.0f));
			else			Mix_HaltMusic();

		}

		if (current_track = music[id])
		{
			if (fade_time)	ret = (Mix_FadeInMusic(current_track, loops, fade_time * 1000) < 0);
			else			ret = (Mix_PlayMusic(current_track, loops) < 0);
		}

	}

	return ret;
}


// Play WAV
bool j1Audio::PlayFx(unsigned int id, uint volume, int repeat)
{

	if (id < fx.size() && active)
	{
		bool tmp = false;
		for (std::list<SFXList>::iterator it = blackList.begin(); it != blackList.end(); it++)
			if (*it == id) { tmp = true; }
			
			if(!tmp)
			{
				blackList.push_back((SFXList)id);
				Mix_VolumeChunk(fx[id], volume*sfxVolumeModifier);
				Mix_PlayChannel(-1, fx[id], repeat);
				return true;
			}
	}

	return false;
}


void j1Audio::LoadMusic(const char* path)
{
	if (active) {

		if (_Mix_Music* track = Mix_LoadMUS(PATH(MUSIC_FOLDER, path)))
			music.push_back(track);
		else 
			LOG("Cannot load music %s. Mix_GetError(): %s", path, Mix_GetError());
	}

}

// Load WAV
void j1Audio::LoadFx(const char* path)
{
	if (active) {

		if (Mix_Chunk* chunk = Mix_LoadWAV(PATH(FX_FOLDER, path)))
			fx.push_back(chunk);
		else
			LOG("Cannot load wav %s. Mix_GetError(): %s", path, Mix_GetError());
	}

}


void j1Audio::LoadFXFromXML()
{
	pugi::xml_document SFXdoc;
	pugi::xml_node file;

	file = App->LoadFile(SFXdoc, "Audio_Paths.xml");

	for (pugi::xml_node SFX = file.child("SFX").child("path"); SFX; SFX = SFX.next_sibling("path"))
		LoadFx(SFX.attribute("sfx").as_string());

	for (pugi::xml_node Music = file.child("Music").child("path"); Music; Music = Music.next_sibling("path"))
		LoadMusic(Music.attribute("track").as_string());
}

void j1Audio::ModifyMusicVolume(float value)
{
	musicVolumeModifier = value;
	if (musicVolumeModifier < 0)musicVolumeModifier = 0;
	else if (musicVolumeModifier > 1)musicVolumeModifier = 1;

	Mix_VolumeMusic(128 * musicVolumeModifier);
}

void j1Audio::ModifySFXVolume(float value)
{
	sfxVolumeModifier = value;
	if (sfxVolumeModifier < 0)sfxVolumeModifier = 0;
	else if (sfxVolumeModifier > 1)sfxVolumeModifier = 1;
}
