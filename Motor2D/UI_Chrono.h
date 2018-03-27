#ifndef __UI_CHRONO__
#define __UI_CHRONO__
#include "UI_element.h"
#include "UI_Text.h"
#include "SDL/include/SDL_pixels.h"
#include "j1Timer.h"

struct _TTF_Font;

enum chrono_type
{
	STOPWATCH,
	TIMER
};

class Chrono : public UI_element
{
public:
	Chrono()
	{}

	Chrono(int x, int y, chrono_type type, _TTF_Font* font, SDL_Color color, j1Module* callback) : UI_element(x, y, element_type::CHRONO, { 0, 0, 0, 0 }, callback, nullptr),
		type(type)
	{
		text = new Text("00000", x, y, font, color, nullptr);
		//counter.Pause();
		time = -1;
	}

	~Chrono()
	{
		delete text;
	}

	void BlitElement(bool use_camera = false);
	void setStartValue(int new_start_value);
	void setAlarm(int alarm);
	void restartChrono();

public:
	int time = 1;
	uint time_elapsed = 0;
	uint start_value = 0;
	std::list<uint> alarms;
	j1Timer counter;
	Text* text;
	chrono_type type;
	std::string last_secs;
};

#endif // !__UI_CHRONO__



