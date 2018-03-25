#include "UI_Text.h"
#include "j1App.h"
#include "j1Textures.h"
#include "j1Fonts.h"
#include "j1Render.h"
#include "Brofiler\Brofiler.h"

Text::~Text()
{
	if (outline != nullptr)
	{
		App->tex->UnLoad(outline);
		outline = nullptr;
	}
}

void Text::createTexture()
{
	if (texture != nullptr)
	{
		App->tex->UnLoad(texture);
		texture = nullptr;
	}
	if (outline != nullptr)
	{
		App->tex->UnLoad(outline);
		outline = nullptr;
	}

	uint outline_width = 0;
	uint outline_height = 0;
	/*if (outlined)
	{
		App->font->setFontOutline(font, 2);
		outline = App->font->Print(text.c_str(), outline_color, font); //Outlined texture
		App->tex->GetSize(outline, outline_width, outline_height);
	}

	App->font->setFontOutline(font, 0);*/
	texture = App->font->Print(text.c_str(), &color, font); //Normal texture
	App->tex->GetSize(texture, tex_width, tex_height);
	section.w = tex_width;
	section.h = tex_height;

	if (outlined)
	{
		outline_offset.x = tex_width - outline_width;
		outline_offset.x /= 2;
		outline_offset.y = outline_offset.x;
	}

}

void Text::setColor(SDL_Color newColor)
{
	color = newColor;
	createTexture();
}

void Text::setOutlineColor(SDL_Color newColor)
{
	outline_color = newColor;
	if (outlined)
		createTexture();
}

void Text::BlitElement()
{
	BROFILER_CATEGORY("Text Blit", Profiler::Color::Fuchsia);

	if (counting)
	{
		if (counter != nullptr)
		{
			std::string newValue = std::to_string(*counter);
			if (newValue != text)
			{
				setText(newValue);
			}
		}
		else
		{
			if (text != "No counter assigned")
				setText("No counter assigned");
		}
	}

	if (texture != nullptr)
	{
		SDL_SetTextureAlphaMod(texture, App->gui->alpha_value);
		iPoint globalPos = calculateAbsolutePosition();

		if (outlined)
		{
			SDL_SetTextureAlphaMod(outline, App->gui->alpha_value);
			App->render->Blit(outline, globalPos.x + outline_offset.x, globalPos.y + outline_offset.y, NULL, false);
		}
		App->render->Blit(texture, globalPos.x, globalPos.y, NULL, false);
	}

	BlitChilds();
}

void Text::setOutlined(bool isOutlined)
{
	if (isOutlined != outlined)
	{
		outlined = isOutlined;
		createTexture();
	}
}

void Text::convertIntoCounter(int* variableCounting)
{
	counting = true;
	counter = variableCounting;
}

std::string Text::getText() const
{
	return text;
}

void Text::setText(std::string string)
{
	text = string;
	createTexture();
}

int Text::getLength() const
{
	return text.length();
}
