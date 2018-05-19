#ifndef __UI_WARNINGMESSAGES_H__
#define __UI_WARNINGMESSAGES_H__

#include "UI_element.h"

enum message_type
{
	NO_WORKERS,
	NO_RESOURCES,
	NO_TREES,
	OUT_OF_RANGE,
	NO_MINE
};

class Text;

class WarningMessages : public UI_element
{
public:
	WarningMessages();
	~WarningMessages();

	void addWarningMessage(std::string message, message_type type);
	void showMessage(message_type type);
	void hideMessage(message_type type);

	void BlitElement();

public:

	std::vector<Text*> warnings;
};

#endif // __UI_WARNINGMESSAGES_H__