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
			unit->piercing_atk = unit->piercing_atk*buff;

			unit->attack = unit->attack*buff;
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