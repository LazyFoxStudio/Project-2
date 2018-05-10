#pragma once
#include "p2Defs.h"

struct Color
{
	uint r, g, b, a;

	Color() : r(0), g(0), b(0), a(255)
	{}

	Color(uint r, uint g, uint b, uint a = 255) : r(r), g(g), b(b), a(a)
	{}

	void Set(uint r, uint g, uint b, uint a = 255)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	float* operator & ()
	{
		return (float*)this;
	}

};
 

static Color Red	= Color(255, 0, 0);
static Color Green	= Color(0, 255, 0);
static Color Blue	= Color(0, 0, 255);
static Color Black	= Color(0, 0, 0);
static Color White	= Color(255, 255, 255);
static Color Grey	= Color(177, 177, 177);
static Color Yellow = Color(255, 255, 102);

static Color Translucid_Red = Color(255, 0, 0, 100);
static Color Translucid_Yellow	= Color(255, 255, 102, 125);
static Color Translucid_Green	= Color(0, 255, 0, 125);
static Color Translucid_Blue	= Color(0, 0, 255, 125);
static Color Translucid_Grey	= Color(125, 125, 125, 125);
static Color Translucid_DarkGrey = Color(50, 50, 50, 200);

static Color Transparent_Green  = Color(0, 255, 0, 75);
static Color Transparent_Red    = Color(255, 0, 0, 75);
static Color Transparent_Blue = Color(0, 255, 255, 40);