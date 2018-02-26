#include "p2Defs.h"
#include "p2Log.h"
#include "p2Animation.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "j1Fonts.h"
#include "j1Input.h"
#include "j1Gui.h"
#include "j1IntroScene.h"
#include "j1Scene.h"
#include "Window.h"
#include "UIElement.h"
#include "Label.h"
#include "Image.h"
#include "j1Audio.h"
#include "Interactive.h"
#include "InteractiveImage.h"
#include "InteractiveLabel.h"
#include "InteractiveLabelledImage.h"
#include "LabelledImage.h"
#include "InheritedImage.h"
#include "InheritedInteractive.h"
#include "InheritedLabel.h"
#include "Scrollbar.h"
#include "LifeBar.h"

j1Gui::j1Gui() : j1Module()
{
	name.create("gui");
}

// Destructor
j1Gui::~j1Gui()
{}

// Called before render is available
bool j1Gui::Awake(pugi::xml_node& conf)
{
	LOG("Loading GUI atlas");
	bool ret = true;

	buttonFX = conf.child("buttonFX").attribute("source").as_string("");
	atlas_file_name = conf.child("atlas").attribute("file").as_string("");
	background = conf.child("background").attribute("file").as_string("");

	return ret;
}

// Called before the first frame
bool j1Gui::Start()
{
	App->audio->LoadFx(buttonFX.GetString());
	//atlas = App->tex->Load(atlas_file_name.GetString());
	App->input->GetMousePosition(mouseLastFrame.x, mouseLastFrame.y);

	return true;
}

// Update all guis
bool j1Gui::PreUpdate()
{
	bool ret = true;

	for (p2List_item<UIElement*>* item = elements.start; item && ret; item = item->next)
	{
		if (item->data->active)
			if (!item->data->In_window || item->data->window->active)
				ret = item->data->PreUpdate();
	}

	if (ret)
	{
		for (p2List_item<Window*>* item = window_list.end; item && ret; item = item->prev)
		{
			if (item->data->active)
				ret = item->data->PreUpdate();
		}
	}

	return ret;
}

// Called after all Updates
bool j1Gui::PostUpdate()
{
	bool ret = true;

	if (App->input->GetKey(SDL_SCANCODE_F8) == KEY_DOWN)
		debug = !debug;

	for (p2List_item<UIElement*>* item = elements.start; item && ret; item = item->next)
	{
		if (item->data->active)
			if (!item->data->In_window || item->data->window->active)
				ret = item->data->PostUpdate();

	}
	
	App->input->GetMousePosition(mouseLastFrame.x, mouseLastFrame.y);
	return ret;
}

bool j1Gui::Draw(float dt)
{
	bool ret = true;
	for (p2List_item<UIElement*>* item = elements.start; item && ret; item = item->next)
	{
		if (item->data->active)
		{
			if (!item->data->In_window || item->data->window->active)
				item->data->Draw(dt);

			if (debug && item->data->active)
				if (!item->data->In_window || item->data->window->active)
				{
					
					if(!item->data->In_window)
					App->render->DrawQuad(item->data->position, Red,true,false);
					else
					{
						SDL_Rect tmp = { item->data->position.x + item->data->winElement->relativePosition.x,item->data->position.y + item->data->winElement->relativePosition.y,item->data->position.w,item->data->position.h };
						App->render->DrawQuad(tmp, Red,true,false );
			
					}
				}

		}
	}

	return ret;
}

// Called before quitting
bool j1Gui::CleanUp()
{
	LOG("Freeing GUI");
	for (p2List_item<UIElement*>* item = elements.start; item; item = item->next) item->data->CleanUp();
	elements.clear();
	return true;
}

bool j1Gui::Save(pugi::xml_node &config) const
{
	/*config.append_child("score").append_attribute("value") = scoreNumber;
	config.append_child("coins").append_attribute("value") = coins;*/
	return true;
}

bool j1Gui::Load(pugi::xml_node &config)
{
	if (App->scene->active)
	{
		//scoreNumber = config.child("score").attribute("value").as_int();
		//if (scoreNumber < 0 || scoreNumber > 100000)
		//{
		//	scoreNumber = 0;
		//}

		//p2SString temp("Score: %i", App->gui->scoreNumber);
		//App->gui->currentScore->ChangeText(&temp);

		//p2SString temo("Coins: %i", App->gui->coins);
		//App->gui->currentCoins->ChangeText(&temo);
		//coins = config.child("coins").attribute("value").as_int();
	}
	return true;
}



InheritedImage* j1Gui::AddImage(SDL_Rect& position, iPoint positionOffset, SDL_Rect& section, bool draggable)
{
	InheritedImage* ret = new InheritedImage(position, positionOffset, section, draggable);
	elements.add(ret);
	return ret;
}

LifeBar* j1Gui::AddLifebar(SDL_Rect& position, iPoint positionOffset, SDL_Rect& section, bool draggable)
{
	LifeBar* ret = new LifeBar(position, positionOffset, section, draggable);
	elements.add(ret);
	return ret;
}



InheritedInteractive * j1Gui::AddInteractive(SDL_Rect & position, iPoint positionOffset, SDL_Rect & size, InteractiveType type, j1Module * callback, bool draggable)
{
	InheritedInteractive* ret = new InheritedInteractive(position, positionOffset, size, type, callback, draggable);
	elements.add(ret);
	return ret;
}

InheritedLabel* j1Gui::AddLabel(SDL_Rect& position, iPoint positionOffset, p2SString fontPath, SDL_Color textColor, p2SString label, int size, bool draggable)
{
	InheritedLabel* ret = new InheritedLabel(position, positionOffset, fontPath, textColor, label, size, draggable);
	elements.add(ret);

	return ret;
}



InteractiveImage * j1Gui::AddInteractiveImage(SDL_Rect & position, iPoint positionOffsetA, iPoint positionOffsetB, SDL_Rect image_section,InteractiveType type, j1Module * callback, bool draggable)
{
	InteractiveImage* ret = new InteractiveImage(position, positionOffsetA, positionOffsetB, image_section, type, callback, draggable);
	elements.add(ret);
	return ret;
}

InteractiveLabel * j1Gui::AddInteractiveLabel(SDL_Rect & position, iPoint positionOffsetA, iPoint positionOffsetB, p2SString fontPath, SDL_Color textColor, p2SString label, int size, InteractiveType type, j1Module * callback, bool draggable)
{
	InteractiveLabel* ret = new InteractiveLabel(position, positionOffsetA, positionOffsetB, fontPath, textColor, label, size, type, callback, draggable);
	elements.add(ret);
	return ret;
}



InteractiveLabelledImage* j1Gui::AddInteractiveLabelledImage(SDL_Rect & position, iPoint positionOffsetA, iPoint positionOffsetB, iPoint positionOffsetC, SDL_Rect & image_section, p2SString & fontPath, SDL_Color & textColor, p2SString & label, int size, InteractiveType type, j1Module * callback, bool draggable)
{
	InteractiveLabelledImage* ret = new InteractiveLabelledImage(position, positionOffsetA, positionOffsetB, positionOffsetC, image_section, fontPath, textColor, label, size, type, callback, draggable);
	elements.add(ret);
	return ret;
}


LabelledImage* j1Gui::AddLabelledImage(SDL_Rect & position, iPoint positionOffsetA, iPoint Imagerelativepos, p2SString fontPath, SDL_Color textColor, p2SString label, int size, SDL_Rect image_section, bool draggable)
{
	LabelledImage* ret = new LabelledImage(position, positionOffsetA, Imagerelativepos, fontPath, textColor, label, size, image_section, draggable);
	elements.add(ret);
	return ret;
}


Scrollbar * j1Gui::AddScrollbar(SDL_Rect & scroller_image, bool moves_vertically, int min, SDL_Rect & pos, iPoint Sliderrelativepos, SDL_Rect image_section, ScrollbarType type, bool draggable)
{
	Scrollbar* ret = new Scrollbar(scroller_image, moves_vertically, min, pos, Sliderrelativepos, image_section, type, draggable);
	elements.add(ret);
	return ret;
}




UIElement* j1Gui::DeleteElement(UIElement* element)
{
	int index = elements.find(element);
	p2List_item<UIElement*>* item = nullptr;
	for (item = elements.start; item; item = item->next)
	{
		if (item->data == element)
		{
			break;
		}
	}
	elements.del(item);

	return item->data;
}

UIElement* j1Gui::AddImage_From_otherFile(SDL_Rect& position, iPoint positionOffset, p2SString &path, bool draggable)
{
	UIElement* element = new InheritedImage(position, positionOffset, path, draggable);

	elements.add(element);

	return element;
}

Window * j1Gui::AddWindow(SDL_Rect &windowrect, bool draggable)
{
	Window* window = new Window(windowrect,draggable);

	window_list.add(window);

	return window;
}

void j1Gui::Load_UIElements(pugi::xml_node node, j1Module* callback)
{
	BROFILER_CATEGORY("UI", Profiler::Color::Chocolate);

}


UIElement* j1Gui::Load_InteractiveLabelledImage_fromXML(pugi::xml_node tmp, j1Module* callback)
{
	InteractiveLabelledImage* added = nullptr;
	
	SDL_Rect pos = { tmp.child("pos").attribute("x").as_int(), tmp.child("pos").attribute("y").as_int(), tmp.child("pos").attribute("w").as_int(), tmp.child("pos").attribute("h").as_int() };
	iPoint relativeposA = { tmp.child("relativeposA").attribute("x").as_int(),tmp.child("relativeposA").attribute("y").as_int() };
	iPoint relativeposB = { tmp.child("relativeposB").attribute("x").as_int(),tmp.child("relativeposB").attribute("y").as_int() };
	iPoint relativeposC = { tmp.child("relativeposC").attribute("x").as_int(),tmp.child("relativeposC").attribute("y").as_int() };
	SDL_Rect section = { tmp.child("imagesection").attribute("x").as_int(), tmp.child("imagesection").attribute("y").as_int(), tmp.child("imagesection").attribute("w").as_int(), tmp.child("imagesection").attribute("h").as_int() };
	p2SString path = (tmp.child("fontpath").attribute("path").as_string());
	SDL_Color color = { tmp.child("color").attribute("r").as_int(), tmp.child("color").attribute("g").as_int(), tmp.child("color").attribute("b").as_int(), tmp.child("color").attribute("a").as_int() };
	p2SString label = (tmp.child("label").attribute("string").as_string());
	int size = tmp.child("size").attribute("value").as_int();
	InteractiveType type = InteractiveType_from_int(tmp.child("type").attribute("value").as_int());
	bool draggable = tmp.child("draggable").attribute("value").as_bool();

	added = AddInteractiveLabelledImage(pos, relativeposA, relativeposB, relativeposC, section, path, color, label, size, type, callback, draggable);
	
	if (!tmp.child("active").attribute("value").as_bool(true))
		added->active = false;

	added->hover = { tmp.child("hover").attribute("x").as_int(), tmp.child("hover").attribute("y").as_int(), tmp.child("hover").attribute("w").as_int(), tmp.child("hover").attribute("h").as_int() };
	added->click = { tmp.child("click").attribute("x").as_int(), tmp.child("click").attribute("y").as_int(), tmp.child("click").attribute("w").as_int(), tmp.child("click").attribute("h").as_int() };
	added->inactive = { tmp.child("inactive").attribute("x").as_int(), tmp.child("inactive").attribute("y").as_int(), tmp.child("inactive").attribute("w").as_int(), tmp.child("inactive").attribute("h").as_int() };

	return added;
}

Window* j1Gui::Load_Window_fromXML(pugi::xml_node node, j1Module* callback)
{
	SDL_Rect collider = { node.child("collider").attribute("x").as_int(), node.child("collider").attribute("y").as_int(), node.child("collider").attribute("w").as_int(), node.child("collider").attribute("h").as_int() };

	bool draggable =  node.child("draggable").attribute("value").as_bool(false);
	Window* added = AddWindow(collider,draggable);
	if (!node.child("active").attribute("value").as_bool(true))
		added->active = false;

	return added;
}



UIElement* j1Gui::Load_Image_fromXML(pugi::xml_node node)
{
	SDL_Rect position = { node.child("position").attribute("x").as_int(), node.child("position").attribute("y").as_int(), node.child("position").attribute("w").as_int(), node.child("position").attribute("h").as_int() };
	iPoint relativePos = { node.child("relativePosition").attribute("x").as_int(),node.child("relativePosition").attribute("y").as_int() };
	SDL_Rect image_section = { node.child("image_section").attribute("x").as_int(), node.child("image_section").attribute("y").as_int(), node.child("image_section").attribute("w").as_int(), node.child("image_section").attribute("h").as_int() };
	bool draggable = node.child("draggable").attribute("value").as_bool();

	if (node.child("lifebar").attribute("value").as_bool(false))
	{
		LifeBar* added = AddLifebar(position, relativePos, image_section, draggable);

		Load_LifeBar_formXML(node, added);

		if (!node.child("active").attribute("value").as_bool(true))
			added->active = false;

		App->scene->lifebar = added;

		return added;
	}
	else
	{
		Image* added = AddImage(position, relativePos, image_section, draggable);

		if (!node.child("active").attribute("value").as_bool(true))
			added->active = false;


		return added;
	}
}

UIElement* j1Gui::Load_AlterantiveImage_fromXML(pugi::xml_node node)
{
	p2SString path = node.child("path").attribute("string").as_string();
	SDL_Rect position = { node.child("position").attribute("x").as_int(), node.child("position").attribute("y").as_int(), node.child("position").attribute("w").as_int(), node.child("position").attribute("h").as_int() };
	iPoint relativePos = { node.child("relativePosition").attribute("x").as_int(),node.child("relativePosition").attribute("y").as_int() };
	bool draggable = node.child("draggable").attribute("value").as_bool();
	UIElement* added = AddImage_From_otherFile(position, relativePos, path, draggable);
	if (!node.child("active").attribute("value").as_bool(true))
		added->active = false;
	return added;
}


UIElement * j1Gui::Load_Label_fromXML(pugi::xml_node node)
{
	InheritedLabel* ret;
	SDL_Rect position = { node.child("position").attribute("x").as_int(), node.child("position").attribute("y").as_int(), node.child("position").attribute("w").as_int(), node.child("position").attribute("h").as_int() };
	iPoint relativepos =  { node.child("relativePosition").attribute("x").as_int(),node.child("relativePosition").attribute("y").as_int() }; 
	p2SString fontPath = node.child("fontpath").attribute("path").as_string();
	SDL_Color color = { node.child("color").attribute("r").as_int(), node.child("color").attribute("g").as_int(), node.child("color").attribute("b").as_int(), node.child("color").attribute("a").as_int() };
	p2SString label = node.child("label").attribute("value").as_string();
	int size = node.child("size").attribute("value").as_int();
	bool draggable = node.child("draggable").attribute("value").as_bool();

	ret = AddLabel(position, relativepos, fontPath, color, label, size, draggable);

	if (!node.child("active").attribute("value").as_bool(true))
		ret->active = false;
	return ret;
}

UIElement * j1Gui::Load_InteractiveImage_fromXML(pugi::xml_node node, j1Module* callback)
{
	InteractiveImage* ret;
	SDL_Rect position = { node.child("position").attribute("x").as_int(), node.child("position").attribute("y").as_int(), node.child("position").attribute("w").as_int(), node.child("position").attribute("h").as_int() };
	iPoint relativeposA = { node.child("relativePositionA").attribute("x").as_int(),node.child("relativePosition").attribute("y").as_int() };
	iPoint relativeposB = { node.child("relativePositionB").attribute("x").as_int(),node.child("relativePosition").attribute("y").as_int() };
	SDL_Rect image_section = { node.child("imagesection").attribute("x").as_int(), node.child("imagesection").attribute("y").as_int(), node.child("imagesection").attribute("w").as_int(), node.child("imagesection").attribute("h").as_int() };
	bool draggable = node.child("draggable").attribute("value").as_bool();
	InteractiveType type = InteractiveType_from_int(node.child("type").attribute("value").as_int());
	ret = AddInteractiveImage(position, relativeposA, relativeposB, image_section, type, callback, draggable);
	ret->hover = { node.child("hover").attribute("x").as_int(), node.child("hover").attribute("y").as_int(), node.child("hover").attribute("w").as_int(), node.child("hover").attribute("h").as_int() };
	ret->click = { node.child("click").attribute("x").as_int(), node.child("click").attribute("y").as_int(), node.child("click").attribute("w").as_int(), node.child("click").attribute("h").as_int() };
	ret->inactive = { node.child("inactive").attribute("x").as_int(), node.child("inactive").attribute("y").as_int(), node.child("inactive").attribute("w").as_int(), node.child("inactive").attribute("h").as_int() };
	
	if (!node.child("active").attribute("value").as_bool(true))
		ret->active = false;
	return ret;
}

UIElement * j1Gui::Load_Scrollbar_fromXML(pugi::xml_node node)
{
	Scrollbar* ret;
	SDL_Rect scroller_image = { node.child("scrollerimage").attribute("x").as_int(), node.child("scrollerimage").attribute("y").as_int(), node.child("scrollerimage").attribute("w").as_int(), node.child("scrollerimage").attribute("h").as_int() };
	bool moves_vertically = node.child("movesvertically").attribute("value").as_bool();
	int min = node.child("min").attribute("value").as_int();
	SDL_Rect position = { node.child("position").attribute("x").as_int(), node.child("position").attribute("y").as_int(), node.child("position").attribute("w").as_int(), node.child("position").attribute("h").as_int() };
	iPoint sliderrelativepos = { node.child("sliderrelativepos").attribute("x").as_int(),node.child("sliderrelativepos").attribute("y").as_int() };
	SDL_Rect image_section = { node.child("imagesection").attribute("x").as_int(), node.child("imagesection").attribute("y").as_int(), node.child("imagesection").attribute("w").as_int(), node.child("imagesection").attribute("h").as_int() };
	ScrollbarType type = ScrollbarType_from_int(node.child("type").attribute("value").as_int(0));
	bool draggable = node.child("draggable").attribute("value").as_bool();
	ret = AddScrollbar(scroller_image, moves_vertically, min, position, sliderrelativepos, image_section,type, draggable);
	if (!node.child("active").attribute("value").as_bool(true))
		ret->active = false;
	return ret;
}

UIElement * j1Gui::Load_LabelledImage_fromXML(pugi::xml_node node)
{
	LabelledImage* ret;
	SDL_Rect position = { node.child("position").attribute("x").as_int(), node.child("position").attribute("y").as_int(), node.child("position").attribute("w").as_int(), node.child("position").attribute("h").as_int() };
	iPoint labeloffset = { node.child("labelrelativepos").attribute("x").as_int(),node.child("labelrelativepos").attribute("y").as_int() };
	iPoint imageoffset = { node.child("imageoffset").attribute("x").as_int(),node.child("imageoffset").attribute("y").as_int() };
	p2SString fontpath = node.child("fontpath").attribute("string").as_string();
	SDL_Color color = { node.child("color").attribute("r").as_int(), node.child("color").attribute("g").as_int(), node.child("color").attribute("b").as_int(), node.child("color").attribute("a").as_int() };
	p2SString label = node.child("label").attribute("string").as_string();
	int size = node.child("size").attribute("value").as_int();
	SDL_Rect imagesection = { node.child("imagesection").attribute("x").as_int(), node.child("imagesection").attribute("y").as_int(), node.child("imagesection").attribute("w").as_int(), node.child("imagesection").attribute("h").as_int() };
	bool draggable = node.child("draggable").attribute("value").as_bool();
	
	ret = AddLabelledImage(position, labeloffset, imageoffset, fontpath, color, label, size, imagesection, draggable);
	if (!node.child("active").attribute("value").as_bool(true))
		ret->active = false;

	if (node.child("isTimeLabel").attribute("value").as_bool(false))
		timeLabel = ret;

	return ret;
}

void j1Gui::Load_LifeBar_formXML(pugi::xml_node node, LifeBar* imageData)
{
	imageData->threeLivesImageSection = { node.child("threeLives").attribute("x").as_int(), node.child("threeLives").attribute("y").as_int(), node.child("threeLives").attribute("w").as_int(), node.child("threeLives").attribute("h").as_int() };
	imageData->twoLivesImageSection = { node.child("twoLives").attribute("x").as_int(), node.child("twoLives").attribute("y").as_int(), node.child("twoLives").attribute("w").as_int(), node.child("twoLives").attribute("h").as_int() };
	imageData->lastLifeImageSection = { node.child("lastLife").attribute("x").as_int(), node.child("lastLife").attribute("y").as_int(), node.child("lastLife").attribute("w").as_int(), node.child("lastLife").attribute("h").as_int() };

}

Animation j1Gui::LoadPushbacks_fromXML(pugi::xml_node node)
{
	Animation ret;
	pugi::xml_node tmp = node.child("pushback");

	while (tmp)
	{
		ret.PushBack({ tmp.attribute("x").as_int(), tmp.attribute("y").as_int(), tmp.attribute("w").as_int(), tmp.attribute("h").as_int() });
		tmp = tmp.next_sibling("pushback");
	}
	
	ret.speed = node.child("speed").attribute("value").as_float();
	ret.loop = node.child("loop").attribute("value").as_bool();
	

	return ret;
}

InteractiveType j1Gui::InteractiveType_from_int(int type)
{
	InteractiveType ret;
	switch (type)
	{
	case(1):
		ret = QUIT;
		break;
	case(2):
		ret = CONTINUE;
		break;
	case(3):
		ret = NEWGAME;
		break;
	case(4):
		ret = OPEN_SETTINGS;
		break;
	case(5):
		ret = OPEN_CREDITS;
		break;
	case(6):
		ret = CLOSE_WINDOW;
		break;	
	case(7):
		ret = EXIT_TO_MENU;
		break;
	case(8):
		ret = SAVE_GAME;
		break;
	case(9):
		ret = UNLOCKUI;
		break;
	default:
		ret = DEFAULT;
	};
	return ret;
}

ScrollbarType j1Gui::ScrollbarType_from_int(int type)
{
	ScrollbarType ret;

	switch (type)
	{
	case(1):
		ret = MUSICVOLUME;
		break;
	case(2):
		ret = SFXVOLUME;
		break;
	default:
		ret = SCROLLBAR_DEFAULT;
		break;
	}
	return ret;
}


// const getter for atlas
 SDL_Texture* j1Gui::GetAtlas() const
{
	return atlas;
}

// class Gui ---------------------------------------------------

 bool j1Gui::BecomeFocus(Window* curr)
 {
	 bool ret = true;
	 p2List_item<Window*>* item = nullptr;
	 for (item = window_list.end; item; item = item->prev)
	 {
		 if (item->data == curr)
		 {
			 RemoveFocuses();
			 break;
		 }
		 if (item->data->hasFocus)
		 {
			 ret = false;
			 break;
		 }
	 }
	 return ret;
 }

 void j1Gui::RemoveFocuses()
 {
	 p2List_item<Window*>* item = nullptr;
	 for (item = window_list.end; item; item = item->prev)
		 {
			 for (p2List_item<WinElement*>* item2 = item->data->children_list.start; item2; item2 = item2->next)
				 item2->data->element->hasFocus = false;

			 item->data->hasFocus = false;
		 }
 }

 bool j1Gui::CheckWindowFocuses()
 {
	 bool ret = false;

	 p2List_item<Window*>* item = nullptr;
	 for (item = window_list.end; item; item = item->prev)
	 {
		 if (item->data->hasFocus)
		 {
			 ret = true;
			 break;
		 }
	 }

	 return ret;
 }

 void j1Gui::WindowlessFocuses()
 {
	 bool focus = false;

	 p2List_item<UIElement*>* item = nullptr;
	 for (item = elements.start; item; item = item->next)
	 {
		 if (item->data->In_window || !item->data->active || item->data->UItype == UIType::IMAGE || item->data->UItype == UIType::LABEL || item->data->UItype == UIType::LABELLED_IMAGE || item->data->UItype == UIType::NO_TYPE || item->data->UItype == UIType::UICLOCK)
			 continue;

		 if (item->data->hasFocus)
		 {
			focus = true;
			break;
		}
	 }

	 if (!focus)
	 {
		 FocusOnFirstElement();
	 }
	 else
	 {
		 p2List_item<UIElement*>* item = nullptr;
		 for (item = elements.start; item; item = item->next)
		 {
			 if (item->data->hasFocus && !item->data->Unavalible)
			 {
				 item->data->hasFocus = false;
				 FocusOnNextElement(item);
				 break;
			 }
		 }
	 }
 }

 void j1Gui::FocusOnFirstElement()
 {
	 p2List_item<UIElement*>* item = nullptr;
	 for (item = elements.start; item; item = item->next)
	 {
		 if (!item->data->In_window && !item->data->Unavalible && item->data->active && item->data->UItype != UIType::IMAGE && item->data->UItype != UIType::LABEL && item->data->UItype != UIType::LABELLED_IMAGE && item->data->UItype != UIType::NO_TYPE && item->data->UItype != UIType::UICLOCK)
		 {
			 item->data->hasFocus = true;
			 break;
		 }
	 }
 }

 void j1Gui::FocusOnNextElement(p2List_item<UIElement*>* item)
 {
	 while (1 == 1)
	 {
		 if (item->next && !item->next->data->Unavalible && (item->next->data->UItype == UIType::INTERACTIVE || item->next->data->UItype == UIType::INTERACTIVE_LABEL || item->next->data->UItype == UIType::INTERACTIVE_IMAGE || item->next->data->UItype == UIType::INTERACTIVE_LABELLED_IMAGE))
		 {
			 item->next->data->hasFocus = true;
			 break;
		 }
		 else if (!item->next)
		 {
			 FocusOnFirstElement();
			 break;
		 }
		 item = item->next;
	 }
 }
