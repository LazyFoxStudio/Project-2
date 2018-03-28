#ifndef __J1CONSOLE_H__
#define __J1CONSOLE_H__

#include "j1Module.h"
#include "p2Point.h"
#include <string>
#include <list>
#include <vector>

struct function
{
	std::string name;
	j1Module* callback;
	int min_args;
	int max_args;
};

class j1Console : public j1Module
{
public:
	j1Console();
	~j1Console();

	bool Start();

	bool PreUpdate();
	bool Update(float dt);
	bool PostUpdate();

	bool CleanUp();

	bool UIinteraction(/*UIelement* element*/);

	bool Save(pugi::xml_node& node)const;
	bool Load(pugi::xml_node& node);

	std::list<std::string> logs;
	
	//UITextbox* input = nullptr;

	std::list<function*> functions;
	int AddFunction(const char*, j1Module* ,int, int);

	std::string function_1;
	std::string exit;
	std::string help;

	bool Console_Interaction(std::string& function, std::vector<int>& arguments);

public:
	bool console_active = false;

};
#endif // !__J1ENTITIES_H__