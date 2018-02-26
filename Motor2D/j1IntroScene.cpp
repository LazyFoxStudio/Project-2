#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Input.h"
#include "j1Textures.h"
#include "j1Audio.h"
#include "j1Render.h"
#include "j1Scene.h"
#include "j1SceneSwitch.h"
#include "j1IntroScene.h"
#include "j1Gui.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "UIElement.h"
#include "InheritedImage.h"
#include "InteractiveLabelledImage.h"
#include "Window.h"



j1IntroScene::j1IntroScene()
{
	name.create("introscene");
}


j1IntroScene::~j1IntroScene()
{
}

bool j1IntroScene::Awake()
{
	return true;
}

bool j1IntroScene::Start()
{
	pugi::xml_document	Gui_config_file;
	pugi::xml_node		guiconfig;

	guiconfig = App->LoadConfig(Gui_config_file, "Gui_config.xml");

	guiconfig = guiconfig.child("introscene");

	// App->audio->PlayMusic("MainTheme.ogg");

	return true;
}

bool j1IntroScene::PreUpdate()
{
	return true;
}

bool j1IntroScene::Update(float dt)
{

	App->gui->Draw(dt);
	if (App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN) return false;

	return true;
}

bool j1IntroScene::PostUpdate()
{
	return true;
}

bool j1IntroScene::CleanUp()
{
	return true;
}
