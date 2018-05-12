#ifndef __UI_SLIDER__
#define __UI_SLIDER__

#include "UI_element.h"
#include "UI_Text.h"

class Button;

enum modifier
{
	NO_MODIFIER,

	MUSIC,
	FX
};

class Slider : public UI_element
{
public:

	Slider()
	{}

	Slider(int x, int y, SDL_Texture* texture, SDL_Rect empty, SDL_Rect full, float default_progress, j1Module* callback);

	~Slider()
	{}

	float getProgress() const;
	void setProgress(float newProgress);
	void BlitElement();

	Button* getButton() const;

public:

	SDL_Rect full;
	Text* progress_num = nullptr;
	float progress = 0.0f;
	int bar_length = 0;
	modifier modify = NO_MODIFIER;
};

#endif // !__UI_SLIDER__