#ifndef __UI_COSTDISPLAY__
#define __UI_COSTDISPLAY__

#include "UI_element.h"


struct _TTF_Font;

class Image;
class Text;

class CostDisplay : public UI_element
{
public:

	CostDisplay(std::string entityname, int cost, resourceType resource);

	~CostDisplay();

public:
	
};



#endif

