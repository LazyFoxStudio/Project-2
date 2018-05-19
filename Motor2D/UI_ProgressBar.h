#ifndef __UI_PROGRESSBAR__
#define _UI_PROGRESSBAR__

#include "UI_element.h"

enum bar_type
{
	INCREASING,
	DECREASING
};

class ProgressBar : public UI_element
{
public:

	ProgressBar(int x, int y, SDL_Texture* texture, SDL_Rect empty, SDL_Rect full, SDL_Rect head, float max_value, j1Module* callback);

	~ProgressBar()
	{}

	void setProgress(float newProgress);
	float getProgress() const;
	//return -1 if new value is lower than previous
	//return 0 if new value is the same than previous
	//return 1 if new value is higher than previous
	int enterCurrentValue(float current_value);
	void BlitElement();

public:

	SDL_Rect full = { 0,0,0,0 };
	float max_value = 0.0f;
	float progress = 1.0f;
	SDL_Rect head = { 0,0,0,0 };
	iPoint head_pos = { 0,0};
	bar_type type = INCREASING;
};

#endif // !__UI_PROGRESSBAR__