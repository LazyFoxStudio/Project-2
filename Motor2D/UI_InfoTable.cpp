#include "UI_InfoTable.h"
#include "UI_Text.h"
#include "j1App.h"
#include "j1Fonts.h"
#include "j1EntityController.h"
#include "Entity.h"

InfoTable::~InfoTable()
{
	cleanInfo();
}

void InfoTable::newSelection()
{
	cleanInfo();
	if (App->entitycontroller->selected_entities.size() > 0)
	{
		iPoint position = INFOTABLE_POSITION;
		InfoData* data = App->entitycontroller->selected_entities.front()->infoData;
		if (data != nullptr)
		{
			title = new Text(data->title, position.x, position.y, App->font->fonts.front(), { 0,0,0,255 }, nullptr);

			int counterY = 1;
			for (std::list<InfoLineData*>::iterator it_l = data->linesData.begin(); it_l != data->linesData.end(); it_l++)
			{
				lines.push_back(new InfoLine((*it_l), position.x + 10, position.y + (counterY*LINES_SEPARATION))); //MEMLEAK
				counterY++;
			}
		}
	}
}

void InfoTable::cleanInfo()
{
	RELEASE(title);

	std::list<InfoLine*>::iterator it_l;
	it_l = lines.begin();
	while (it_l != lines.end())
	{
		RELEASE((*it_l));
		it_l++;
	}
	lines.clear();
}

void InfoTable::BlitElement()
{
	if (title != nullptr)
		title->BlitElement();

	for (std::list<InfoLine*>::iterator it_l = lines.begin(); it_l != lines.end(); it_l++)
	{
		(*it_l)->Draw();
	}
}

InfoLine::InfoLine(const InfoLineData* data, int x, int y)
{
	type = data->type;
	text = new Text(data->text, x, y, App->font->fonts.front(), { 0,0,0,255 }, nullptr);
	if (type == STAT)
		value = new Text(std::to_string(data->value), x + STATS_VALUE_OFFSET, y, App->font->fonts.front(), { 0,0,0,255 }, nullptr);
}

void InfoLine::Draw()
{
	text->BlitElement();
	if (type == STAT)
		value->BlitElement();
}
