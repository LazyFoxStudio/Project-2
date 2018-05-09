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
	std::string help;
};

class j1Console : public j1Module
{
public:
	j1Console();
	~j1Console() {};

	bool Start();

	bool PreUpdate();
	bool Update(float dt);
	bool PostUpdate();

	bool CleanUp();

	bool Usefunction(/*UIelement* element*/);

	bool Save(pugi::xml_node& node)const;
	bool Load(pugi::xml_node& node);

	std::list<std::string> logs;
	
	//text being written in the console
	std::string editable_text;

	std::list<function*> functions;

	function* AddFunction(const char*, j1Module* ,int, int, const char* = "");

	function* help;

	bool Console_Interaction(std::string& function, std::vector<int>& arguments);

public:
	bool console_active = false;
private:
	int textpos = 0;

};
#endif // !__J1ENTITIES_H__
