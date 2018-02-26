#pragma once
#include "Label.h"
class InheritedLabel : public Label
{
public:
	InheritedLabel();
	InheritedLabel(SDL_Rect& position, iPoint positionOffset, p2SString fontPath, SDL_Color textColor, p2SString& label, int size, bool draggable);
	~InheritedLabel();

	bool PreUpdate();
	bool PostUpdate();
	bool Draw(float dt);
	bool CleanUp();
};

