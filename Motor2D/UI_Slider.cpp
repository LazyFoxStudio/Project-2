#include "UI_Slider.h"
#include "j1App.h"
#include "j1Render.h"
#include "UI_Button.h"
#include "Brofiler\Brofiler.h"
#include "j1Fonts.h"

Slider::Slider(int x, int y, SDL_Texture * texture, SDL_Rect empty, SDL_Rect full, float default_progress, j1Module * callback) : UI_element(x, y, SLIDER, empty, callback, texture),
full(full),
bar_length(full.w),
progress(default_progress)
{
	progress_num = new Text("", x, y, App->font->getFont(1), { 255,255,255,255 }, nullptr);
}

float Slider::getProgress() const
{
	return progress;
}

void Slider::setProgress(float newProgress)
{
	progress = newProgress;
}

void Slider::BlitElement()
{
	BROFILER_CATEGORY("Slider Blit", Profiler::Color::LemonChiffon);

	iPoint globalPos = calculateAbsolutePosition();
	App->render->Blit(texture, globalPos.x, globalPos.y, &section, false, true);

	std::string newText = std::to_string(progress*100);
	progress_num->setText(newText);
	progress_num->localPosition.x = full.w * App->gui->w_stretch - progress_num->section.w / (2 / App->gui->w_stretch);
	progress_num->localPosition.y = -progress_num->section.h * App->gui->h_stretch;

	if (progress < 0.01)
		progress = 0;
	else if (progress > 1)
		progress = 1;
	full.w = ((bar_length)* progress);
	App->render->Blit(texture, globalPos.x, globalPos.y, &full, false, true);

	progress_num->BlitElement();

	UI_element::BlitElement();
}