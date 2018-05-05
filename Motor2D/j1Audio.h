#ifndef __j1AUDIO_H__
#define __j1AUDIO_H__

#include "j1Module.h"

#include <vector>

#define DEFAULT_MUSIC_FADE_TIME 2
#define MUSIC_FOLDER "audio/music/"
#define FX_FOLDER "audio/fx/"
#define DEFAULT_MUSIC_MODIFIER 1
#define DEFAULT_SFX_MODIFIER 1

struct _Mix_Music;
struct Mix_Chunk;

class j1Audio : public j1Module
{
public:

	j1Audio();

	// Destructor
	virtual ~j1Audio();

	// Called before render is available
	bool Awake(pugi::xml_node&);

	// Called before quitting
	bool CleanUp();

	bool Save(pugi::xml_node&) const;
	bool Load(pugi::xml_node&);


	bool j1Audio::PlayMusic(uint id, uint fade_time = DEFAULT_MUSIC_FADE_TIME);

	void LoadFx(const char* path);
	void LoadMusic(const char* path);

	void LoadFXFromXML();

	// Play a previously loaded WAV
	bool PlayFx(unsigned int fx, uint volume = 128, int repeat = 0);

	void ModifyMusicVolume(int value);
	void ModifySFXVolume(int value);



private:

	float						musicVolumeModifier = 1;
	float						sfxVolumeModifier = 1;
	_Mix_Music*					current_track = nullptr;
	std::vector<Mix_Chunk*>		fx;
	std::vector<_Mix_Music*>	music;

public:

};

enum MusicList
{
	MAIN_THEME,
	INGAME_THEME,
	DEFEAT_THEME
};

enum SFXList
{
	//Peasant SFX
	SFX_PEASANT_READY,
	SFX_PEASANT_WAITING_FOR_ORDERS,
	SFX_PEASANT_PISSED,
	SFX_PEASANT_FOLLOWING_ORDERS,
	SFX_PEASANT_FINISHED_JOB,

	//Archer SFX
	SFX_ARCHER_READY,
	SFX_ARCHER_WAITING_FOR_ORDERS,
	SFX_ARCHER_FOLLOWING_ORDERS,
	SFX_ARCHER_PISSED,

	//Footman SFX
	SFX_FOOTMAN_READY,
	SFX_FOOTMAN_WAITING_FOR_ORDERS,
	SFX_FOOTMAN_FOLLOWING_ORDERS,
	SFX_FOOTMAN_PISSED,

	//Knight SFX 
	SFX_KNIGHT_READY,
	SFX_KNIGHT_WAITING_FOR_ORDERS,
	SFX_KNIGHT_FOLLOWING_ORDERS,
	SFX_KNIGHT_PISSED,

	//Griffon SFX 
	SFX_GRIFFON_READY,
	SFX_GRIFFON_WAITING_FOR_ORDERS,
	SFX_GRIFFON_FOLLOWING_ORDERS,
	SFX_GRIFFON_PISSED,

	//Flying Machine SFX 
	SFX_FLYING_MACHINE_READY,
	SFX_FLYING_MACHINE_WAITING_FOR_ORDERS,
	SFX_FLYING_MACHINE_FOLLOWING_ORDERS,
	SFX_FLYING_MACHINE_PISSED,

	//Yha'mam SFX
	SFX_HERO_YHAMAM_BASICATTACK,
	SFX_HERO_YHAMAM_ICICLECRASH,
	SFX_HERO_YHAMAM_OVERFLOW,
	SFX_HERO_YHAMAM_DRAGONBREATH,

	//Miscellanious SFX
	SFX_MISCELLANEOUS_SWORD_CLASH,
	SFX_MISCELLANEOUS_AXETHROW,
	SFX_MISCELLANEOUS_ARROW,
	SFX_MISCELLANEOUS_BALLISTA,
	SFX_MISCELLANEOUS_CATAPULT,
	SFX_MISCELLANEOUS_GRYPHON,

	//UI
	SFX_BUTTON_CLICKED, // Isn't yet on the xml (temporary Map assist sfx)
	SFX_MAP_ASSIST,
	SFX_MAP_ATTACK
};
#endif // __j1AUDIO_H__