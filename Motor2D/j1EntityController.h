#ifndef __j1ENTITYCONTROLLER_H__
#define __j1ENTITYCONTROLLER_H__

#include "j1Module.h"
#include "Building.h"
#include "Unit.h"
#include "Hero.h"
#include "j1Audio.h"
#include "j1Console.h"

#include <list>
#include <map>
#include <queue>

class Squad;
class Quadtree;

#define TOWN_HALL_POS {2000,2000}
#define MAX_VILLAGERS_LUMBERMILL 10
#define MAX_VILLAGERS_FARM 5
#define FARM_WORKER_PRODUCTION_SECONDS 10
#define DEATH_TIME 5
#define WOOD_PER_WORKER 2
#define GOLD_PER_WORKER 2.2
#define REPAIR_COST 400
#define REPAIR_COOLDOWN 30
#define UNIT_QUEUE_MAX_SIZE 6
#define BUILDINGAREA 1750
#define MATCHUP_MODIFIER 1.5

#define MAGE_ABILITY_1_COOLDOWN 6
#define MAGE_ABILITY_2_COOLDOWN 8
#define MAGE_ABILITY_3_COOLDOWN 4

#define PALADIN_ABILITY_1_COOLDOWN 3
#define PALADIN_ABILITY_2_COOLDOWN 10
#define PALADIN_ABILITY_3_COOLDOWN 4

#define PALADIN_BUFF_DURATION 15
#define PALADIN_ARMOR_REDUCTION -5
#define PALADIN_DAMAGE_REDUCTION -10
#define PALADIN_DAMAGE_BUFF 7
#define PALADIN_ARMOR_BUFF 10

//------------Upgrades section----------
#define ATTACK_UPGRADE_GROWTH 2
#define DEFENSE_UPGRADE_GROWTH 2


#define MELEE_2_UPGRADE_COST 1000


#define RANGED_2_UPGRADE_COST 1200


#define FLYING_2_UPGRADE_COST 2000

enum UpgradeType
{
	NO_TYPE,
	MELEE_ATTACK_UPGRADE,
	MELEE_DEFENSE_UPGRADE,
	RANGED_ATTACK_UPGRADE,
	RANGED_DEFENSE_UPGRADE,
	FLYING_ATTACK_UPGRADE,
	FLYING_DEFENSE_UPGRADE
};
struct worker
{
	Building* farm = nullptr;
	Building* working_at = nullptr;
	bool to_destroy = false;

	worker(Building* _farm)
	{
		farm = _farm;
		working_at = nullptr;
		to_destroy = false;
	}
};

struct Forest
{
	std::list<iPoint> trees;
};

struct Order
{
	bool operator()(const Entity* entity_1, const Entity* entity_2)const
	{
		return  entity_1->GetPriority() > entity_2->GetPriority();
	}
};

class j1EntityController : public j1Module
{
public:
	j1EntityController();
	~j1EntityController() {};

	bool Start();
	bool Update(float dt);
	bool PostUpdate();
	bool CleanUp();

	bool Save(pugi::xml_node&) const;
	bool Load(pugi::xml_node&);

	bool loadEntitiesDB(pugi::xml_node& data);
	float getSpeedFromAtkSpeed(float atk_speed, int anim_frames);

	
	void DeleteEntity(Entity* entity);
	void DeleteSquad(Squad* squad);
	void DeleteDB();

	Unit* addUnit(iPoint pos, Type type, Squad* squad = nullptr);
	Hero* addHero(iPoint pos, Type type);
	Building* addBuilding(iPoint pos, Type type);
	Squad* AddSquad(Type type, fPoint position);

	void selectionControl();
	void commandControl();

	void buildingCalculations();
	bool placeBuilding(iPoint position);
	void buildingProcessDraw();

	void HandleWorkerAssignment(bool to_assign, Building* building);
	bool CheckCost(Type type);
	Cost getCost(Type type);
	bool SpendCost(Type type);
	void debugDrawEntity(Entity* entity);
	SFXList GetOrdersSFXFromType(Type type);
	void HandleAttackSFX(Type type, int volume = 128);
	void HandleReadySFX(Type type, int volume = 128);
	void HandleOrdersSFX();
	void HandleParticles(Type type, fPoint pos, fPoint obj, float speed = 300);
	void GetTotalIncome();
	void TownHallLevelUp();

	Entity* getEntitybyID(uint ID);
	Squad* getSquadbyID(uint ID);

	Unit* getUnitFromDB(Type type);
	Hero* getHeroFromDB(Type type);
	Building* getBuildingFromDB(Type type);

	Entity* CheckMouseHover(iPoint mouse_world);
	void CheckCollidingWith(SDL_Rect collider, std::vector<Entity*>& list_to_fill, Entity* entity_to_ignore = nullptr);
	Entity* getNearestEnemy(Entity* entity, int squad_target, Entity* current_enemy = nullptr);
	bool getNearestEnemies(Entity* entity, int squad_target, int number, std::vector<Entity*>& list_to_fill);

	bool ChechUpgradeCost(UpgradeType type) const;
	void SpendUpgradeResources(UpgradeType type);
	void UpgradeUnits(UpgradeType type);
	void LoadUpgrades(int m_dmg, int m_armor, int r_dmg, int r_armor, int f_dmg, int f_armor);
	void UpgradeExistingUnits(Type type1, Type type2, UpgradeType up_type);
	Cost getUpgradeCost(UpgradeType type);

	void RefundResources(Type type);
	//------Worker Related Functions--------
	void SubstractRandomWorkers(int num);

	void CreateWorkers(Building* target, int num);

	bool CheckInactiveWorkers(int num = 1);
	worker* GetInactiveWorker();
	void AssignWorker(Building* building, worker* worker);
	void DestroyWorkers();
	void UnassignRandomWorker();

	bool CreateForest(MapLayer* trees);
	bool Console_Interaction(std::string& function, std::vector<int>& arguments);


public:

	std::list<Entity*> entities;
	std::list<Entity*> selected_entities;
	std::list<Entity*> operative_entities;

	std::list<Squad*> squads;
	std::list<Squad*> selected_squads;


	std::map<uint, Entity*> DataBase;

	std::list<Forest*> forests;

	std::priority_queue <Entity*, std::vector<Entity*>, Order> SpriteQueue;

	uint last_UID = 0;
	uint hero_UID = -1;

	Building* town_hall = nullptr;
/*
	j1Timer time_slicer;
	std::list<Entity*>::iterator entity_iterator;
	std::list<Squad*>::iterator squad_iterator;*/

	bool debug = false;
	bool blit_skill = false;

	Quadtree* colliderQT = nullptr;

	Type to_build_type = NONE_ENTITY;
	SDL_Rect selection_rect = { 0,0,0,0 };
	SDL_Rect buildingArea;

	int m_dmg_lvl = 0;
	int m_armor_lvl = 0;
	int r_dmg_lvl = 0;
	int r_armor_lvl = 0;
	int f_dmg_lvl = 0;
	int f_armor_lvl = 0;

	bool hasBuilt_LumberMill	= false;
	bool hasBuilt_Barracks		= false;
	bool hasBuilt_Farm			= false;
	bool hasBuilt_Mine			= false;
	bool hasBuilt_GnomeHut		= false;

	function* lose_game;
	function* reset_hero_cd;
	function* new_worker_cost;
	function* new_wood_cost;
	function* new_gold_cost;
	function* new_oil_cost;
	function* complete_buildings;
	function* kill_selected;
	function* change_stat;
	function* next_wave;
	function* spawn_squad;
	function* spawn_building;
};
#endif // !

