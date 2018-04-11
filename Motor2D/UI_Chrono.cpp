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
	switch (this->type)
	{
	case TIMER:
		time = start_value;
		break;
	case STOPWATCH:
		time = 0;
		break;
	}
}

void Chrono::BlitElement(bool use_camera)
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

				if (last_secs != secs)
					text->setText(secs);

				section = text->section;
				last_secs = secs;
			}
			break;
		}

		text->BlitElement();

		UI_element::BlitElement(use_camera);
	}
}