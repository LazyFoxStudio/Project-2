#include "Effects.h"
#include "Unit.h"

#include <math.h>

//regular damage
void DamageBuff::Apply()
{
	switch (sign)
	{
	case PLUS_MINUS:
	{unit->attack += buff;
	break;}
	case MULTIPLICATION_DIVISION:
	{float now = unit->attack;
	unit->attack = round(now*buff);
	break;}
	}
}
void DamageBuff::Remove()
{
	switch (sign)
	{
	case PLUS_MINUS:
	{unit->attack -= buff;
	break;}
	case MULTIPLICATION_DIVISION:
	{float now = unit->attack;
	unit->attack = round(now/buff);
	break;}
	}
}
//piercing_damage
void PiercingDamageBuff::Apply()
{
	switch (sign)
	{
	case PLUS_MINUS:
	{unit->piercing_atk += buff;
	break;}
	case MULTIPLICATION_DIVISION:
	{float now = unit->piercing_atk;
	unit->piercing_atk = round(now*buff);
	break;}
	}
}
void PiercingDamageBuff::Remove()
{
	switch (sign)
	{
	case PLUS_MINUS:
	{unit->piercing_atk -= buff;
	break;}
	case MULTIPLICATION_DIVISION:
	{float now = unit->piercing_atk;
	unit->piercing_atk = round(now/buff);
	break;}
	}
}
//speed boost
void SpeedBuff::Apply()
{
	switch (sign)
	{
	case PLUS_MINUS:
	{unit->speed += buff;
	break; }
	case MULTIPLICATION_DIVISION:
	{float now = unit->speed;
	unit->speed = round(now*buff);
	break; }
	}
}
void SpeedBuff::Remove()
{
	switch (sign)
	{
	case PLUS_MINUS:
	{unit->speed -= buff;
	break; }
	case MULTIPLICATION_DIVISION:
	{float now = unit->speed;
	unit->speed = round(now / buff);
	break; }
	}
}
//defense boost
void DefenseBuff::Apply()
{
	switch (sign)
	{
	case PLUS_MINUS:
	{unit->defense += buff;
	break; }
	case MULTIPLICATION_DIVISION:
	{float now = unit->defense;
	unit->defense = round(now*buff);
	break; }
	}
}
void DefenseBuff::Remove()
{
	switch (sign)
	{
	case PLUS_MINUS:
	{unit->defense -= buff;
	break; }
	case MULTIPLICATION_DIVISION:
	{float now = unit->defense;
	unit->defense = round(now / buff);
	break; }
	}
}
//range boost
void RangeBuff::Apply()
{
	switch (sign)
	{
	case PLUS_MINUS:
	{unit->range += buff;
	break; }
	case MULTIPLICATION_DIVISION:
	{float now = unit->range;
	unit->range = round(now*buff);
	break; }
	}
}
void RangeBuff::Remove()
{
	switch (sign)
	{
	case PLUS_MINUS:
	{unit->range -= buff;
	break; }
	case MULTIPLICATION_DIVISION:
	{float now = unit->range;
	unit->range = round(now / buff);
	break; }
	}
}