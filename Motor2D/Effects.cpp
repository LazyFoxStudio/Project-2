#include "Effects.h"
#include "Unit.h"


void DamageBuff::Apply() 
{ 
	switch (sign)
	{
		case PLUS_MINUS:
		{
			unit->attack += buff;
			break;
		}
		case MULTIPLICATION_DIVISION:
		{
			LOG("OLD P %d", unit->piercing_atk);
			unit->piercing_atk = unit->piercing_atk*buff;
			LOG("NEW P %d",unit->piercing_atk);

			LOG("OLD A %d", unit->attack);
			unit->attack = unit->attack*buff;
			LOG("NEW A %d", unit->attack);
			break;
		}
	}
}
void DamageBuff::Remove() 
{ 
	switch (sign)
	{
	case PLUS_MINUS:
	{
		unit->attack -= buff;
		break;
	}
	case MULTIPLICATION_DIVISION:
	{
		unit->attack = unit->attack/buff;

		unit->piercing_atk = unit->piercing_atk/buff;

		break;
	}
	}
}