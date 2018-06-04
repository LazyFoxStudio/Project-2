#ifndef __j1AUDIO_H__
#define __j1AUDIO_H__

#include "j1Module.h"
#include "j1Timer.h"
#include <vector>
#include <list>

#define DEFAULT_MUSIC_FADE_TIME 2
#define MUSIC_FOLDER "audio/music/"
#define FX_FOLDER "audio/fx/"
#define DEFAULT_MUSIC_MODIFIER 1
#define DEFAULT_SFX_MODIFIER 0.5f

struct _Mix_Music;
struct Mix_Chunk;

enum SFXList
{
	//Peasant SFX
	SFX_PEASANT_READY,//Psready
	SFX_PEASANT_WAITING_FOR_ORDERS,//Pswhat1
	SFX_PEASANT_PISSED,//Pspissd7
	SFX_PEASANT_FOLLOWING_ORDERS,//Psyessr4
	SFX_PEASANT_FINISHED_JOB,//Pswrkdon

	//Archer SFX
	SFX_ARCHER_READY,//Eready
	SFX_ARCHER_WAITING_FOR_ORDERS,//Ewhat3
	SFX_ARCHER_FOLLOWING_ORDERS,//Eyessir3
	SFX_ARCHER_PISSED,//Epissed3

	//Footman SFX
	SFX_FOOTMAN_READY,//Hready
	SFX_FOOTMAN_WAITING_FOR_ORDERS,//Hwhat3
	SFX_FOOTMAN_FOLLOWING_ORDERS,//Hyessir3
	SFX_FOOTMAN_PISSED,//Hpissed3

	//Knight SFX 
	SFX_KNIGHT_READY,//Knready
	SFX_KNIGHT_WAITING_FOR_ORDERS,//Knwhat3
	SFX_KNIGHT_FOLLOWING_ORDERS,//Knyessr3
	SFX_KNIGHT_PISSED,//Knpissd3

	//Gryphon SFX 
	SFX_GRYPHON_READY,//Griffon2
	SFX_GRYPHON_WAITING_FOR_ORDERS,//Dwhat1
	SFX_GRYPHON_FOLLOWING_ORDERS,//Dwyessr3
	SFX_GRYPHON_PISSED,//Dwpissd3

	//Flying Machine SFX 
	SFX_FLYING_MACHINE_READY,//Gnready
	SFX_FLYING_MACHINE_WAITING_FOR_ORDERS,//Gnpissd1
	SFX_FLYING_MACHINE_FOLLOWING_ORDERS,//Gnyessr1
	SFX_FLYING_MACHINE_PISSED,//Gnpissd3

	//Yha'mam SFX
	SFX_HERO_YHAMAM_BASICATTACK,//Spell
	SFX_HERO_YHAMAM_ICICLECRASH,//Icestorm
	SFX_HERO_YHAMAM_OVERFLOW,//Blodlust
	SFX_HERO_YHAMAM_DRAGONBREATH,//Flamshld

	//Miscellanious SFX
	SFX_MISCELLANEOUS_SWORD_CLASH,//Sword3
	SFX_MISCELLANEOUS_AXETHROW,//Axe
	SFX_MISCELLANEOUS_ARROW,//Bowfire
	SFX_MISCELLANEOUS_BALLISTA,//Ballfire
	SFX_MISCELLANEOUS_FLYINGMACHINE,//Thunk
	SFX_MISCELLANEOUS_CATAPULT,//Catapult
	SFX_MISCELLANEOUS_GRYPHON,//Thunder
	SFX_MISCELLANEOUS_DEATH_KNIGHT,//Iokilrog
	SFX_MISCELLANEOUS_DRAGON,//Burning
	SFX_MISCELLANEOUS_JUGGERNAUT,//Explode
	SFX_MISCELLANEOUS_THHEAL,//Heal
	SFX_MISCELLANEOUS_CANCEL_TROOP, //Cancel Troop


	//UI
	SFX_CANTDOTHAT,//Cantdothat
	SFX_BUTTON_CLICKED, // Isn't yet on the xml (temporary Map assist sfx)
	SFX_MAP_ASSIST,//Massist
	SFX_MAP_ATTACK,//Mattack
	SFX_TUTORIAL_STEP,//Tutorial_step
	SFX_UNLOCK_BUILDING,//UnlockBuilding

	//HERO PALADIN
	SFX_CONSECRATION,//Invisibl
	SFX_CIRCLE_OF_LIGHT,//Holyvisn
	SFX_HONOR_OF_THE_PURE,//Exorcism

	SFX_NO_SFX
};



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

	bool PostUpdate();

	bool Save(pugi::xml_node&) const;
	bool Load(pugi::xml_node&);


	bool j1Audio::PlayMusic(uint id, uint fade_time = DEFAULT_MUSIC_FADE_TIME, int loops = -1);

	void LoadFx(const char* path);
	void LoadMusic(const char* path);

	void LoadFXFromXML();

	// Play a previously loaded WAV
	bool PlayFx(unsigned int fx, uint volume = 128, int repeat = 0);

	void ModifyMusicVolume(float value);
	void ModifySFXVolume(float value);



private:

	j1Timer						blackListCooldown;
	std::list<SFXList>			blackList;
	float						musicVolumeModifier = 1;
	float						sfxVolumeModifier = 1;
	_Mix_Music*					current_track = nullptr;
	std::vector<Mix_Chunk*>		fx;
	std::vector<_Mix_Music*>	music;

public:
	bool canDefeat = false;
	j1Timer followOrdersCooldown;

};

enum MusicList
{
	MAIN_THEME,
	INGAME_THEME,
	DEFEAT_THEME,
	LOGO_THEME
};

#endif // __j1AUDIO_H__