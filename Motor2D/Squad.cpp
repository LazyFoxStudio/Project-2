#include "Squad.h"
#include "Unit.h"
#include "Command.h"
#include "j1EntityController.h"

#include "j1Input.h"
#include "j1Render.h"

Squad::Squad(std::vector<uint>& units) : units_id(units)
{
	std::vector<Unit*> squad_units;
	getUnits(squad_units);

	if (!squad_units.empty())
	{
		max_speed = squad_units[0]->speed;
		commander_pos = squad_units[0]->position;

		for (int i = 0; i < squad_units.size(); i++)
		{
			squad_units[i]->squad = this;
			if (squad_units[i]->speed < max_speed) max_speed = squad_units[i]->speed;
		}

		calculateOffsets();
	}
	timer.Start();
}

Squad::~Squad()
{
	commands.clear();
}

bool Squad::Update(float dt)
{
	if (units_id.empty()) { Destroy(); return false; }
	else 
	{
		if (!commands.empty())
		{
			commands.front()->Execute(dt);
			if (commands.front()->state == FINISHED) commands.pop_front();

			if (!squad_movement.IsZero())
			{
				std::vector<Unit*> units;
				getUnits(units);

				if (!units.empty())
				{
					if (timer.ReadSec() > 0.5f)
					{
						calculateAttackSlots();
						timer.Start();
					}

					bool everyone_in_position = true;
					for(int i = 0; i < units.size(); i++)
						if (units[i]->position.DistanceTo(units[i]->mov_target) > SQUAD_UNATTACH_DISTANCE)
							{ everyone_in_position = false; break; }

					commander_pos = units[0]->position + (everyone_in_position ? squad_movement : squad_movement * MIN_NEXT_STEP_MULTIPLIER);
					squad_direction = squad_movement.Normalized();
				}
			}
		}
		else squad_movement = { 0.0f,0.0f };
	}

	return true;
}

void Squad::removeUnit(uint unit_ID)
{
	for (int i = 0; i < units_id.size(); i++)
		if (units_id[i] == unit_ID)
		{
			units_id.erase(units_id.begin() + i);
			units_offsets.erase(units_offsets.begin() + i);
			calculateOffsets();
			return;
		}
}

fPoint Squad::getOffset(uint unit_UID)
{
	if (!units_offsets.empty())
	{
		for (int i = 0; i < units_id.size(); i++)
		{
			if (units_id[i] == unit_UID)
			{
				float angle = squad_direction.GetAngle();
				return { units_offsets[i].x * cos(angle) - units_offsets[i].y * sin(angle) , units_offsets[i].y * cos(angle) + units_offsets[i].x * sin(angle) };
			}
		}
	}
	return { 0,0 };
}

void Squad::calculateOffsets()
{
	std::vector<Unit*> squad_units;
	getUnits(squad_units);

	int radius = 1;
	int counter = 1;

	if (!squad_units.empty())
	{
		units_offsets.clear();
		units_offsets.push_back(fPoint(0.0f, 0.0f));
		switch (formation)
		{
		case SQUARE:

			while (counter < squad_units.size())
			{
				for (int i = -radius; i <= radius && counter < squad_units.size(); i++)
					for (int j = -radius; j <= radius && counter < squad_units.size(); j++)
						if (std::abs(i) == radius || j == std::abs(radius))
						{
							units_offsets.push_back({ (i * squad_units[counter]->collider.w) * 1.2f, (j * squad_units[counter]->collider.h) * 1.2f });
							counter++;
						}
				radius++;
			}
			
			break;
		default:
			break;
		}
	}

	calculateAttackSlots();
}

bool Squad::isInSquadSight(fPoint position)
{
	std::vector<Unit*> squad_units;
	getUnits(squad_units);

	for (int i = 0; i < squad_units.size(); i++)
		if (position.DistanceTo(squad_units[i]->position) < squad_units[i]->line_of_sight) return true;

	return false;
}

bool Squad::findAttackSlots(std::vector<iPoint>& list_to_fill, int target_squad_UID)
{
	list_to_fill.clear();

	if (Unit* commander = getCommander())
	{
		bool isEnemy = commander->IsEnemy();

		for (std::list<Squad*>::iterator it = App->entitycontroller->squads.begin(); it != App->entitycontroller->squads.end(); it++)
		{
			if (target_squad_UID != -1 ? target_squad_UID == (*it)->UID : true)
			{
				if (Unit* enemy_commander = (*it)->getCommander())
				{
					if (enemy_commander->IsEnemy() != isEnemy && !(commander->IsMelee() && enemy_commander->IsFlying()))
					{
						for (std::list<iPoint>::iterator it2 = (*it)->atk_slots.begin(); it2 != (*it)->atk_slots.end(); it2++)
						{
							iPoint world_p = App->map->MapToWorld((*it2).x, (*it2).y);
							if (isInSquadSight({ (float)world_p.x, (float)world_p.y }))
								list_to_fill.push_back(world_p);
						}
					}
				}
			}
		}

		if (isEnemy)
		{
			for (std::list<Entity*>::iterator it = App->entitycontroller->entities.begin(); it != App->entitycontroller->entities.end(); it++)
			{
				if ((*it)->IsBuilding())
				{
					if (isInSquadSight((*it)->position))
						list_to_fill.push_back({ (int)(*it)->position.x, (int)(*it)->position.x });
				}
			}
		}
	}

	return !list_to_fill.empty();
}


void Squad::Halt()
{
	std::vector<Unit*> squad_units;
	getUnits(squad_units);

	for (int i = 0; i < squad_units.size(); i++)
		squad_units[i]->Halt();


	for (std::deque<Command*>::iterator it = commands.begin(); it != commands.end(); it++)
		(*it)->Restart();  // Restarting the order calls onStop(), which would be otherwise unaccesible

	commands.clear();
}

void Squad::getUnits(std::vector<Unit*>& list_to_fill)
{
	std::vector<uint> to_erase;

	for (int i = 0; i < units_id.size(); i++)
	{
		if (Unit* unit = (Unit*)App->entitycontroller->getEntitybyID(units_id[i]))	list_to_fill.push_back(unit);
		else																		to_erase.push_back(units_id[i]);
	}

	for (int j = 0; j < to_erase.size(); j++)
		removeUnit(to_erase[j]);
}


Unit* Squad::getCommander()
{
	std::vector<uint> to_erase;
	Unit* ret = nullptr;

	for (int i = 0; i < units_id.size(); i++)
	{
		if (Unit* unit = (Unit*)App->entitycontroller->getEntitybyID(units_id[i])) { ret = unit; break; }
		else																		to_erase.push_back(units_id[i]);
	}

	for (int j = 0; j < to_erase.size(); j++)
		removeUnit(to_erase[j]);

	return ret;
}

bool ComparePoints(iPoint p1, iPoint p2)
{ return (p1 == p2); }

void Squad::calculateAttackSlots()
{
	atk_slots.clear();
	std::vector<Unit*> squad_units;
	std::vector<iPoint> units_map_p;
	getUnits(squad_units);

	for (int i = 0; i < squad_units.size(); i++)
	{
		if (squad_units[i]->isActive == false || squad_units[i]->ex_state == DESTROYED) 
			continue;

		iPoint map_p = App->map->WorldToMap(squad_units[i]->position.x, squad_units[i]->position.y);
		atk_slots.push_back({map_p.x + 1, map_p.y});
		atk_slots.push_back({map_p.x - 1, map_p.y});
		atk_slots.push_back({map_p.x, map_p.y + 1});
		atk_slots.push_back({map_p.x, map_p.y - 1});
		units_map_p.push_back(map_p);
	}

	atk_slots.unique(ComparePoints);

	for (std::list<iPoint>::iterator it = atk_slots.begin(); it != atk_slots.end(); it++)
	{
		for (int i = 0; i < units_map_p.size(); i++)
			if (units_map_p[i] == *it) { atk_slots.erase(it); it--; break; }
	}
}

bool Squad::isFlying()
{
	if (Unit* commander = getCommander())
		return commander->IsFlying();
	
	return true;
}

void Squad::Destroy()
{
	Halt();
	units_id.clear();
	App->entitycontroller->selected_squads.remove(this);
}