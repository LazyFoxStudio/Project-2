#ifndef __UI_PROGRESSBAR__
#define _UI_PROGRESSBAR__

#include "UI_element.h"

struct marker
{
	Image* active = nullptr;
	Image* unactive = nullptr;
	iPoint position;
};

enum bar_type
{
	INCREASING,
	DECREASING
};

class ProgressBar : public UI_element
{
public:

	ProgressBar(int x, int y, SDL_Texture* texture, SDL_Rect empty, SDL_Rect full, Image* head, j1Module* callback);

	~ProgressBar()
	{}

	void addMarker(int x, int y, Image* active, Image* unactive);
	void setProgress(float newProgress);
	float getProgress() const;
	void enterCurrentValue(float current_value);
	void BlitElement();

public:

	SDL_Rect full;
	float max_value = 0.0f;
	float progress = 0.0f;
	std::list<marker*> markers;
	std::list<Image*> highlights;
	Image* head = nullptr;
	bar_type type;
};

#endif // !__UI_PROGRESSBAR__