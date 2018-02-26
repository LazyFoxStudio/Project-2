#ifndef __LABELLED_IMAGE_H__
#define __LABELLED_IMAGE_H__

#include "Label.h"
#include "Image.h"

class LabelledImage : public Label, public Image
{
public:
	LabelledImage(SDL_Rect & pos, iPoint posOffsetA, iPoint posOffsetB, p2SString fontPath, SDL_Color textColor, p2SString label, int size, SDL_Rect image_section, bool draggable);

	~LabelledImage();

	bool PreUpdate();
	bool PostUpdate();
	bool Draw(float dt);
	bool CleanUp();
};

#endif