#include "j1Console.h"
#include "j1Gui.h"
#include "j1App.h"
#include "p2Log.h"
#include "j1Input.h"
#include "j1Audio.h"
#include "j1Textures.h"
#include "j1Render.h"
#include "j1Window.h"
#include "j1Fonts.h"
#include "Color.h"
#include "Brofiler\Brofiler.h"

j1Console::j1Console()
{
	name = "Console";
}

bool j1Console::Start()
{
	help = AddFunction("help", this, 0, 0);
	help_units = AddFunction("help_units", this, 0, 0);
	help_buildings = AddFunction("help_buildings", this, 0, 0);

	return true;
}

bool j1Console::CleanUp()
{
	BROFILER_CATEGORY("CleanUp_Console", Profiler::Color::DarkOliveGreen);
	logs.clear();
	for (std::list<function*>::iterator it = functions.begin(); it != functions.end(); it++)
	{
		RELEASE(*it);
	}
	functions.clear();

	return true;
}

bool j1Console::PreUpdate()
{
	BROFILER_CATEGORY("PreUpdate_Console", Profiler::Color::DarkOliveGreen);
	
	if (App->input->GetKey(SDL_SCANCODE_C) == KEY_DOWN && App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT)
	{
		if (!console_active)
		{
			LOG("console on!");
			console_active = true;
			App->input->StartTextInput();
		}
		else
		{
			LOG("console off!");
			console_active = false;
			App->input->StopTextInput();
		}
	}
	return true;
}

bool j1Console::Update(float dt)
{
	BROFILER_CATEGORY("Update_Console", Profiler::Color::DarkOliveGreen);
	//TEXT MANAGEMENT

	if (console_active)
	{
		if (App->input->text_buffer.length() > 0)
		{
		editable_text.insert(textpos, App->input->text_buffer);
		textpos += App->input->text_buffer.length();
		App->input->text_buffer.clear();
		LOG("%s", editable_text.c_str());
		}

		if (App->input->GetKey(SDL_SCANCODE_HOME) == KEY_DOWN && editable_text.length() >0)
		{
			textpos = 0;
		}

		if (App->input->GetKey(SDL_SCANCODE_END) == KEY_DOWN && editable_text.length() >0)
		{
			textpos = editable_text.length();
		}

		if (App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_DOWN && editable_text.length() >0 && textpos > 0)
		{textpos -= 1;}

		if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_DOWN && editable_text.length() >0 && textpos < editable_text.length())
		{textpos += 1;}

		if (App->input->GetKey(SDL_SCANCODE_BACKSPACE) == KEY_DOWN && editable_text.length() >0 && textpos > 0)
		{
			editable_text.erase(textpos - 1);
			textpos -= 1;
		}
		if (App->input->GetKey(SDL_SCANCODE_DELETE) == KEY_DOWN && editable_text.length() >0 && textpos != editable_text.length())
		{
			editable_text.erase(textpos);
		}

		if (App->input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN)
		{
			Usefunction();
			textpos = 0;
		}
	}

	return true;
}


bool j1Console::PostUpdate()
{
	BROFILER_CATEGORY("PostUpdate_Console", Profiler::Color::DarkOliveGreen);
	//PRINTING ONLY
	if (console_active)
	{
		SDL_Rect r = { -App->render->camera.x,-App->render->camera.y,App->win->screen_surface->w ,App->win->screen_surface->h / 2 };
		//SDL_Rect r = { 0,0,App->win->screen_surface->w ,App->win->screen_surface->h / 2 };

		App->render->DrawQuad(r, Translucid_Grey,true,true);

		SDL_Color color = { Black.r, Black.g, Black.b, Black.a };

		int iterations = 0;
		for(std::list<std::string>::reverse_iterator it = logs.rbegin(); it != logs.rend(); it++)
		{
			SDL_Texture* text_texture = App->font->Print((*it).c_str(), &color, App->font->default_font);

			App->render->Blit(text_texture, -App->render->camera.x, App->win->screen_surface->h / 2 - 30 - 20*iterations - App->render->camera.y);
			App->tex->UnLoad(text_texture);

			++iterations;
		}
		if (editable_text.length() > 0)
		{
			SDL_Texture* edit_text = App->font->Print(editable_text.c_str(), &color, App->font->default_font);

			App->render->Blit(edit_text, -App->render->camera.x, App->win->screen_surface->h / 2 - App->render->camera.y);
			App->tex->UnLoad(edit_text);

			int width = 0;
			int height;
			std::string yolo = editable_text;
			if (editable_text.length() > 0)
			{
				char* test = (char*)yolo.c_str();
				test[textpos] = '\0';
				App->font->CalcSize(test, width, height, App->font->default_font);
			}
			App->render->DrawQuad({ -App->render->camera.x + width, App->win->screen_surface->h / 2 - App->render->camera.y,2,25 }, Grey, true, true);
		}
	}

	return true;
}

bool j1Console::Usefunction(/*UIelement* element*/)
{
	bool ret = true;

	std::string new_string = editable_text;
	editable_text.clear();

	LOG(new_string.c_str());
	logs.push_back(new_string);
	//we gonna need to analyze this string

	std::string maybe;
	maybe = new_string;

	std::vector<std::string> strings;
	std::string first = "";
	strings.push_back(first);
	//every space in this will be starting a new string
	
	int current_string = 0;
	for (int i = 0; i < new_string.length(); ++i)
	{
		if (maybe[i] == 32)//space
		{
			std::string next_word = "";
			strings.push_back(next_word);
			++current_string;
		}
		else
		{
			strings[current_string] += maybe[i];
		}
	}
		//now we check if he inputed a function
	bool found_function = false;
	for(std::list<function*>::iterator it = functions.begin(); it != functions.end(); it++)
	{
		if ((*it)->name == strings[0] )
		{
			found_function = true;
			std::vector<int> arguments;

			for (int i = 1; i < strings.size(); ++i)
				arguments.push_back(std::stoi(strings[i]));

			if (arguments.size() >= (*it)->min_args && arguments.size() <= (*it)->max_args)
				ret = (*it)->callback->Console_Interaction((*it)->name, arguments);
			else
				logs.push_back("argument error (not enough or too many)");
		}
	}

	if (!found_function) logs.push_back("function not found");
	return ret;
}

function* j1Console::AddFunction(const char* name, j1Module* callback, int min_args, int max_args, const char* _help)
{
	function* new_func = new function();

	new_func->name = name;
	new_func->min_args = min_args;
	new_func->max_args = max_args;
	new_func->callback = callback;
	new_func->help = _help;

	functions.push_back(new_func);

	return new_func;

}

bool j1Console::Console_Interaction(std::string& _function, std::vector<int>& arguments)
{
	if (_function == help->name.c_str())
	{
		logs.push_back("The avalible commands are the following:");
		for (std::list<function*>::iterator it = functions.begin(); it != functions.end(); it++)
		{
			std::string help = "";
			if ((*it)->help == "")
				help = (*it)->name ;
			else
				help = (*it)->name + " (" + (*it)->help + ")";

			logs.push_back(help.c_str());
			LOG("%s", (*it)->name.c_str());
		}
		logs.push_back("");
	}

	if (_function == help_units->name.c_str())
	{
		logs.push_back("The units numbers are:");
		//logs.push_back("1 : TOWN_HALL");
		//logs.push_back("2 : BARRACKS");
		//logs.push_back("3 : LUMBER_MILL");
		//logs.push_back("4 : FARM");
		//logs.push_back("5 : MINE");
		//logs.push_back("6 : TURRET");
		//logs.push_back("7 : GNOME_HUT");
		//logs.push_back("8 : CHURCH");
		//logs.push_back("9 : BLACKSMITH");
		logs.push_back("10 : HERO_1");
		logs.push_back("11 : HERO_2");
		logs.push_back("12 : FOOTMAN");
		logs.push_back("13 : ARCHER");
		logs.push_back("14 : KNIGHT");
		logs.push_back("15 : GRYPHON");
		logs.push_back("16 : BALLISTA");
		logs.push_back("17 : FLYING_MACHINE");
		logs.push_back("18 : GRUNT");
		logs.push_back("19 : AXE_THROWER");
		logs.push_back("20 : DEATH_KNIGHT");
		logs.push_back("21 : DRAGON");
		logs.push_back("22 : CATAPULT");
		logs.push_back("23 : JUGGERNAUT");
	}
	if (_function == help_buildings->name.c_str())
	{
		logs.push_back("The buildings numbers are:");
		logs.push_back("1 : TOWN_HALL");
		logs.push_back("2 : BARRACKS");
		logs.push_back("3 : LUMBER_MILL");
		logs.push_back("4 : FARM");
		logs.push_back("5 : MINE");
		logs.push_back("6 : TURRET");
		logs.push_back("7 : GNOME_HUT");
		logs.push_back("8 : CHURCH");
		logs.push_back("9 : BLACKSMITH");
		//logs.push_back("10 : HERO_1");
		//logs.push_back("11 : HERO_2");
		//logs.push_back("12 : FOOTMAN");
		//logs.push_back("13 : ARCHER");
		//logs.push_back("14 : KNIGHT");
		//logs.push_back("15 : GRYPHON");
		//logs.push_back("16 : BALLISTA");
		//logs.push_back("17 : FLYING_MACHINE");
		//logs.push_back("18 : GRUNT");
		//logs.push_back("19 : AXE_THROWER");
		//logs.push_back("20 : DEATH_KNIGHT");
		//logs.push_back("21 : DRAGON");
		//logs.push_back("22 : CATAPULT");
		//logs.push_back("23 : JUGGERNAUT");
	}
	return true;
}

bool j1Console::Save(pugi::xml_node& node) const
{
	BROFILER_CATEGORY("Save_Console", Profiler::Color::DarkOliveGreen);


	return true;
}

bool j1Console::Load(pugi::xml_node& node)
{
	BROFILER_CATEGORY("Load_Console", Profiler::Color::DarkOliveGreen);
	
	return true;
}