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

		for (int i = 0; i < squad_units.size(); i++)
		{
			squad_units[i]->squad = this;
			if (squad_units[i]->speed < max_speed) max_speed = squad_units[i]->speed;
		}

		calculateCentroid();
		calculateOffsets();
	}
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
			fPoint last_pos = centroid;
			calculateCentroid();
			squad_movement = centroid - last_pos;

			commands.front()->Execute(dt);
			if (commands.front()->state == FINISHED) commands.pop_front();
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
			calculateCentroid();
			calculateOffsets();
			return;
		}
}

void Squad::calculateCentroid()
{
	std::vector<Unit*> squad_units;
	getUnits(squad_units);

	if (!squad_units.empty())
	{
		fPoint positions = { 0.0f, 0.0f };
		for (int i = 0; i < squad_units.size(); i++)
		{
			positions += squad_units[i]->position;
		}

		centroid = positions.Normalized() * (positions.GetModule() / squad_units.size());
	}
}

fPoint Squad::getOffset(uint unit_UID)
{
	if (!units_offsets.empty())
	{
		for (int i = 0; i < units_id.size(); i++)
		{
			if (units_id[i] == unit_UID) return units_offsets[i];
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
							units_offsets.push_back(fPoint((i * squad_units[counter]->collider.w) * 1.2f, (j * squad_units[counter]->collider.h) * 1.2f));
							counter++;
						}
				radius++;
			}
			
			break;
		default:
			break;
		}
	}

	fPoint offset_average = { 0.0f,0.0f};
	for (int i = 0; i < units_offsets.size(); i++)
		offset_average += units_offsets[i];

	offset_average = offset_average.Normalized() * (offset_average.GetModule() / (float)units_offsets.size());

	for (int i = 0; i < units_offsets.size(); i++)
		units_offsets[i] -= offset_average;
}

bool Squad::isInSquadSight(fPoint position)
{
	std::vector<Unit*> squad_units;
	getUnits(squad_units);

	for (int i = 0; i < squad_units.size(); i++)
		if (position.DistanceTo(squad_units[i]->position) < squad_units[i]->line_of_sight) return true;

	return false;
}

bool Squad::getEnemiesInSight(std::vector<uint>& list_to_fill, int target_squad_UID)
{
	list_to_fill.clear();

	if (!units_id.empty())
	{
		if (Unit* commander = getCommander())
		{
			bool isEnemy = commander->IsEnemy();

			for (std::list<Entity*>::iterator it = App->entitycontroller->entities.begin(); it != App->entitycontroller->entities.end(); it++)
			{
				if ((*it)->isActive && isEnemy != (*it)->IsEnemy() && (*it)->ex_state != DESTROYED)
					if (isInSquadSight((*it)->position))
					{
						if ((*it)->IsUnit())
						{
							if (target_squad_UID != -1 ? target_squad_UID != ((Unit*)*it)->squad->UID : false)
								continue;
						}

						list_to_fill.push_back((*it)->UID);
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

void Squad::Destroy()
{
	Halt();
	units_id.clear();
	App->entitycontroller->selected_squads.remove(this);
}