#ifndef __UI_INFOTABLE_H__
#define __UI_INFOTABLE_H__

#include "UI_element.h"

#define INFOTABLE_POSITION {1111, 813}
#define LINES_SEPARATION 40
#define TABLE_WIDTH 245
#define TABLE_HEIGHT 254
#define STATS_VALUE_OFFSET 170
#define DEFAULT_INFO_COLOR {255,255,255,255}

class Text;
struct InfoLineData;

enum line_type
{
	INFO,
	STAT
};

struct InfoLineData
{
	InfoLineData(line_type type, std::string text, int value = 0): type(type), text(text), value(value)
	{}

	line_type type = INFO;
	std::string text = "";
	int value = 0;
};

struct InfoLine
{
	InfoLine(const InfoLineData* data, int x, int y);
	~InfoLine()
	{
		RELEASE(text);
		RELEASE(value);
	}

	void Draw();

	line_type type = INFO;
	Text* text = nullptr;
	Text* value = nullptr;
};

struct InfoData
{
	~InfoData()
	{
		linesData.clear();
		title.clear();
	}

	std::string title = "";
	std::list<InfoLineData*> linesData;
};

class InfoTable : public UI_element
{
public:

	InfoTable() : UI_element(0, 0, element_type::WINDOW, { 0,0,0,0 }, nullptr)
	{}
	~InfoTable();

	void newSelection();
	void cleanInfo();

	void BlitElement();

public:

	Text* title = nullptr;
	std::list<InfoLine*> lines;
};

#endif //__UI_INFOTABLE_H__