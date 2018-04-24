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
	function_1 = AddFunction("name1",this,1,2);
	exit = AddFunction("exit", this, 0, 0);
	help = AddFunction("help", this, 0, 0);

	return true;
}

bool j1Console::CleanUp()
{
	BROFILER_CATEGORY("CleanUp_Console", Profiler::Color::DarkOliveGreen);
	logs.clear();
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
		editable_text += App->input->text_buffer;
		App->input->text_buffer.clear();
		LOG("%s",editable_text.c_str());
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

		App->render->DrawQuad(r, Grey,true,true);

		SDL_Color color = { Black.r, Black.g, Black.b, Black.a };

		int iterations = 0;
		for(std::list<std::string>::reverse_iterator it = logs.rbegin(); it != logs.rend(); it++)
		{
			SDL_Texture* text_texture = App->font->Print((*it).c_str(), &color, App->font->default_font);

			App->render->Blit(text_texture, 0, App->win->screen_surface->h / 2 - 15 - 20*iterations);
			App->tex->UnLoad(text_texture);

			++iterations;
		}
	}

	return true;
}

bool j1Console::Usefunction(/*UIelement* element*/)
{
	bool ret = true;
	if (false/*element == input*/)
	{
		//UITextbox* textbox = (UITextbox*)element;
		std::string new_string = ""; // textbox->text;
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

				if (arguments.size() < (*it)->min_args && arguments.size() > (*it)->max_args)
					ret = (*it)->callback->Console_Interaction((*it)->name, arguments);
				else
					logs.push_back("argument error (not enough or too many)");
			}
		}

		if (!found_function) logs.push_back("function not found");

		//so fukin dirty :V
		//App->gui->delete_element(textbox);
		//input = (UITextbox*)App->gui->GUIAdd_textbox(0, App->win->screen_surface->h / 2, this);
	}

	return ret;
}

int j1Console::AddFunction(const char* name, j1Module* callback, int min_args, int max_args)
{
	function* new_func = new function();

	new_func->name = name;
	new_func->min_args = min_args;
	new_func->max_args = max_args;
	new_func->callback = callback;

	functions.push_back(new_func);

	return functions.size() - 1;

}

bool j1Console::Console_Interaction(std::string& function, std::vector<int>& arguments)
{
	if (function == function_1)
		LOG("works");
	else if (function == exit)
	{
		LOG("Exiting from console");
		return false;
	}
	else if (function == help)
	{
		logs.push_back("The avalible commands are the following:");
		for (std::list<std::string>::iterator it = logs.begin(); it != logs.end(); it++)
		{
			logs.push_back(*it);
			LOG("%s", (*it).c_str());
		}
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