#ifndef __LABEL_H__
#define __LABEL_H__

#include "UIElement.h"
#include "j1Fonts.h"

class Label : virtual public UIElement
{
public:
	bool LabelPreUpdate();
	bool LabelUpdate(float dt);
	bool LabelPostUpdate();
	bool LabelDraw(float dt);
	bool LabelCleanUp();

	void ChangeText(p2SString* newText);

	Label();

	Label(SDL_Rect& position, iPoint positionOffset, p2SString fontPath, SDL_Color textColor, p2SString label, int size = 12);

	~Label();

public:
	iPoint Labelrelativepos = { 0,0 };
	p2SString label;
	_TTF_Font* font = nullptr;
	SDL_Texture* fontTexture;
	SDL_Color textColor;
};

#endif