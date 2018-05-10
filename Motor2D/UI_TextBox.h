#ifndef __UI_TEXTBOX_H__
#define __UI_TEXTBOX_H__

#include "UI_Element.h"
#include <list>

class Text;

#define DEFAULT_COLOR Translucid_DarkGrey

#define BOX_MARGIN 5

class TextBox : public UI_element
{
public:
	TextBox(int x, int y, int w, int h) : UI_element(x, y, TEXT, { x, y, w, h }, nullptr)
	{}
	~TextBox();

	void BlitElement();

	void addTextLine(std::string text);
	int getLinesHeight();
	void changeColor(Color color);

public:

	Color color = DEFAULT_COLOR;
	std::list<Text*> textLines;
};

#endif //__UI_TEXTBOX_H__