#include "UI_ProgressBar.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Gui.h"
#include "UI_Image.h"
#include "Brofiler\Brofiler.h"

ProgressBar::ProgressBar(int x, int y, SDL_Texture* texture, SDL_Rect empty, SDL_Rect full, SDL_Rect head, float max_value, j1Module* callback) : UI_element(x, y, PROGRESSBAR, empty, callback, texture),
full(full),
head(head),
head_pos({ x,y }),
max_value(max_value),
progress(1.0f)
{}

void ProgressBar::setProgress(float newProgress)
{
	progress = newProgress;
}

float ProgressBar::getProgress() const
{
	return progress;
}

int ProgressBar::enterCurrentValue(float current_value)
{
	int ret = 0;

	float newProgress;
	if (current_value == 0)
		newProgress = 0.0f;
	else if (current_value <= max_value)
		newProgress = current_value / max_value;
	else
		newProgress = 1.0f;

	if (newProgress != progress)
	{
		if (newProgress < progress)
			ret = -1;
		else
			ret = 1;
		progress = newProgress;
	}
	
	return ret;
}

void ProgressBar::BlitElement()
{
	if (active)
	{
		BROFILER_CATEGORY("Progress Bar Blit", Profiler::Color::Tan);

		iPoint globalPos = calculateAbsolutePosition();
		App->render->Blit(texture, globalPos.x, globalPos.y, &section, use_camera, true);

		float bar_start;
		float bar_end;
		switch (type)
		{
		case INCREASING:
			full.w = section.w*progress;
			bar_start = 0;
			bar_end = full.w;
			App->render->Blit(texture, globalPos.x, globalPos.y, &full, use_camera, true);
			if (!(head.w == 0 || head.h == 0))
			{
				head_pos.x = (int)(full.w) - (int)(head.w) / 2;
				if (head_pos.x < 1)
					head_pos.x = 1;
				if (head_pos.x + head.w > section.w)
					head_pos.x = section.w - head.w - 2;
				App->render->Blit(texture, head_pos.x, head_pos.y, &head, use_camera, true);
			}
			break;
		case DECREASING:
			LOG("ProgressBar progress: %f", progress);
			full.w = section.w*(1.0f - progress);
			full.x = section.w - full.w;
			bar_start = (section.w) - (full.w);
			bar_end = section.w;
			App->render->Blit(texture, globalPos.x + (section.w) - (full.w), globalPos.y, &full, use_camera, true);
			if (!(head.w == 0 || head.h == 0))
			{
				head_pos.x = (int)(section.w) - (int)(full.w) - 3;
				if (head_pos.x < (section.w) - head.w / 1.5f)
					App->render->Blit(texture, head_pos.x, head_pos.y, &head, use_camera, true);
			}
			break;
		}

		UI_element::BlitElement();
	}
}
