#include "UI_WarningMessages.h"
#include "j1App.h"
#include "j1Fonts.h"
#include "UI_Text.h"

WarningMessages::WarningMessages()
{
	interactive = false;
}


WarningMessages::~WarningMessages()
{
	for (int i = 0; i < warnings.size(); i++)
	{
		RELEASE(warnings.at(i));
	}
}

void WarningMessages::addWarningMessage(std::string message, message_type type)
{
	if (warnings.size() > type)
	{
		RELEASE(warnings.at(type));
	}

	Text* text = new Text(message, 0, 0, App->font->fonts.front(), { 255,0,0, 255 }, nullptr);
	text->setBackground(true, { 50, 50, 50, 130 });
	text->active = false;
	text->interactive = false;

	warnings.push_back(text);//Should be inserted in type position
}

void WarningMessages::showMessage(message_type type)
{
	active = true;
	if (warnings.size() > type)
	{
		warnings.at(type)->active = true;
		if (warnings.at(type)->section.w > section.w)
			section.w = warnings.at(type)->section.w;
	}
}

void WarningMessages::hideMessage(message_type type)
{
	section.w = 0;
	int counter = 0;
	for (int i = 0; i < warnings.size(); i++)
	{
		if (i == type)
			warnings.at(i)->active = false;
		else if (warnings.at(i)->active)
		{
			counter++;
			if (warnings.at(i)->section.w > section.w)
				section.w = warnings.at(i)->section.w;
		}
	}
	if (counter == 0)
		active = false;
}

void WarningMessages::BlitElement()
{
	if (!App->isPaused())
	{
		App->gui->moveElementToMouse(this);

		int counterY = 1;
		for (int i = 0; i < warnings.size(); i++)
		{
			if (warnings.at(i)->active)
			{
				warnings.at(i)->localPosition.x = localPosition.x;
				warnings.at(i)->localPosition.y = localPosition.y - (warnings.at(i)->section.h*counterY);
				warnings.at(i)->BlitElement();
				counterY++;
			}
		}
	}
}
