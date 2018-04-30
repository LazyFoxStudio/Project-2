#include "UI_Chrono.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Gui.h"
#include "Brofiler\Brofiler.h"

void Chrono::setStartValue(int new_start_value)
{
	start_value = new_start_value;
}

void Chrono::setAlarm(int alarm)
{
	alarms.push_back(alarm);
}

void Chrono::restartChrono()
{
	counter.Start();
	if (type == TIMER)
	{
		text->setColor(default_color);
	}
}

void Chrono::Blink()
{
	if (last_blink == 0)
	{
		last_blink = counter.Read();
	}
	if (counter.Read() - last_blink > blinkTime)
	{
		if (text->color.r == 255 && text->color.g == 255 && text->color.b == 255)
			text->setColor({ 255,0,0,255 });
		else
			text->setColor(default_color);
		last_blink = counter.Read();
	}
}

void Chrono::BlitElement()
{
	if (active)
	{
		BROFILER_CATEGORY("Chrono Blit", Profiler::Color::AntiqueWhite);
		time_elapsed = counter.ReadSec();

		switch (type)
		{
		case STOPWATCH:
			if (time != time_elapsed)
			{
				time = time_elapsed;

				if (callback != nullptr) //If has callback send event
				{
					for (int i = 0; i < alarms.size(); i++)
					{
						/*if (time == (int)*alarms.At(i))
							callback->OnUIEvent(this, STOPWATCH_ALARM);*/
					}
				}
				int min = time / 60;
				int sec = time - (min * 60);
				std::string secs = ((min<10)? "0": "") + std::to_string(min) + ":" + ((sec<10) ? "0" : "") + std::to_string(sec);

				if (last_secs != secs)
					text->setText(secs);

				section = text->section;
				last_secs = secs;
			}
			break;
		case TIMER:			
			if (start_value - time_elapsed != time && time != 0)
			{
				time = start_value - time_elapsed;

				if (time == 0 && callback != nullptr) //If has callback send event
					callback->OnUIEvent(this, TIMER_ZERO);

				int min = time / 60;
				int sec = time - (min * 60);
				std::string secs = ((min<10) ? "0" : "") + std::to_string(min) + ":" + ((sec<10) ? "0" : "") + std::to_string(sec);

				if (time <= 10 && time > 5)
					text->setColor({ 255,0,0,255 });

				if (last_secs != secs)
					text->setText(secs);

				section = text->section;
				last_secs = secs;
			}
			if (time <= 5)
				Blink();
			break;
		}

		text->BlitElement();

		UI_element::BlitElement();
	}
}