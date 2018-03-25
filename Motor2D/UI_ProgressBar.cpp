#include "UI_ProgressBar.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Gui.h"
#include "UI_Image.h"
#include "Brofiler\Brofiler.h"

ProgressBar::ProgressBar(int x, int y, SDL_Texture* texture, SDL_Rect empty, SDL_Rect full, SDL_Rect head, j1Module* callback) : UI_element(x, y, PROGRESSBAR, empty, callback, texture),
full(full),
head(head),
head_pos({x,y})
{}

void ProgressBar::addMarker(int x, int y, Image* active, Image* unactive)
{
	marker* newMarker = new marker();
	newMarker->position = { x, y };
	active->localPosition = { x, y };
	active->parent = this;
	unactive->localPosition = { x, y };
	unactive->parent = this;
	newMarker->active = active;
	newMarker->unactive = unactive;
	markers.push_back(newMarker);
}

void ProgressBar::setProgress(float newProgress)
{
	progress = newProgress;
}

float ProgressBar::getProgress() const
{
	return progress;
}

void ProgressBar::enterCurrentValue(float current_value)
{
	if (current_value == 0)
		progress = 0.0f;
	else if (current_value <= max_value)
		progress = current_value / max_value;
}

void ProgressBar::BlitElement()
{
	BROFILER_CATEGORY("Progress Bar Blit", Profiler::Color::Tan);

	iPoint globalPos = calculateAbsolutePosition();
	App->render->Blit(texture, globalPos.x, globalPos.y, &section);
	
	float bar_start;
	float bar_end;
	switch (type)
	{
	case INCREASING:
		full.w = section.w*progress;
		bar_start = 0;
		bar_end = full.w;
		App->render->Blit(texture, globalPos.x, globalPos.y, &full);
		head_pos.x = (int)(full.w) - (int)(head.w) / 2;
		if (head_pos.x < 1)
			head_pos.x = 1;
		if (head_pos.x + head.w > section.w)
			head_pos.x = section.w - head.w - 2;
		App->render->Blit(texture, head_pos.x, head_pos.y, &head);
		break;
	case DECREASING:
		full.w = section.w*(1.0f-progress);
		full.x = section.w - full.w;
		bar_start = (section.w) - (full.w);
		bar_end = section.w;
		App->render->Blit(texture, globalPos.x + (section.w) - (full.w), globalPos.y, &full);
		head_pos.x = (int)(section.w) - (int)(full.w) - 3;
		if (head_pos.x < (section.w) - head.w/1.5f)
			App->render->Blit(texture, head_pos.x, head_pos.y, &head);
		break;
	}

	for (std::list<Image*>::iterator it_i = highlights.begin(); it_i != highlights.end(); it_i++)
	{
		(*it_i)->BlitElement();
	}

	for (std::list<marker*>::iterator it_m = markers.begin(); it_m != markers.end(); it_m++)
	{
		if (((*it_m)->position.x + (*it_m)->active->section.w/2) > bar_start && ((*it_m)->position.x + (*it_m)->active->section.w / 2) < bar_end) //In the full-bar area
		{
			(*it_m)->active->BlitElement();
		}
		else
		{
			(*it_m)->unactive->BlitElement();
		}
	}
	
	BlitChilds();
}
