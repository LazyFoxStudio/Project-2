#ifndef __UI_TEXT__
#define __UI_TEXT__

#include "UI_element.h"
#include "SDL/include/SDL_pixels.h"
#include <string>

struct _TTF_Font;

class Text : public UI_element
{
public:

	Text()
	{}

	Text(std::string text, int x, int y, _TTF_Font* font, SDL_Color color, j1Module* callback) : UI_element(x, y, element_type::TEXT, {0, 0, 0, 0}, callback, nullptr),
		text(text),
		font(font),
		color(color)
	{
		createTexture();
	}

	~Text();

	void createTexture();
	void setColor(SDL_Color newColor);
	void setOutlineColor(SDL_Color newColor);
	void setOutlined(bool isOutlined);
	void setBackground(bool hasBackground, SDL_Color color = { 0, 0, 0, 255 });
	void convertIntoCounter(int* variableCounting);

	std::string getText() const;
	void setText(std::string string);
	int getLength() const;

	void setPrefix(std::string prefix);
	void setSufix(std::string sufix);

	void BlitElement();

public:

	SDL_Color color;
	uint tex_width=0;
	uint tex_height=0;

private:

	std::string text = "";
	std::string prefix = "";
	std::string sufix = "";
	bool hasPrefix = false;
	bool hasSufix = false;
	_TTF_Font* font = nullptr;
	bool outlined = false;
	SDL_Texture* outline = nullptr;
	SDL_Color outline_color = { 0, 0, 0, 255 };
	iPoint outline_offset = { 0,0 };
	bool hasBackground = false;
	SDL_Color background_color = { 0, 0, 0, 255 };
	bool counting = false;
	const int* counter = nullptr;
};

#endif // !__UI_TEXT__
