#include "p2Log.h"
#include "j1App.h"
#include "j1Window.h"
#include "j1Textures.h"
#include "j1Render.h"
#include "j1UIScene.h"
#include "j1Video.h"
#include "j1Input.h"
#include "j1Audio.h"

j1Video::j1Video()
{
	name = "video";
}

j1Video::~j1Video()
{
}

bool j1Video::Awake(pugi::xml_node & node)
{

	folder = node.child("folder").attribute("value").as_string();
	
	SDL_ShowCursor(SDL_DISABLE);

	return true;
}

bool j1Video::CleanUp()
{
	App->video->CloseAVI();
	return true;
}

void j1Video::Initialize(char* path)
{
	std::string real_path = folder+path;
	OpenAVI(real_path.c_str());                  // Open The AVI File
}

void j1Video::OpenAVI(LPCSTR path)
{
	AVIFileInit();                          // Opens The AVIFile Library

	if (AVIStreamOpenFromFile(&pavi, path, streamtypeVIDEO, 0, OF_READ, NULL) != 0) // Opens The AVI Stream
	{
		LOG("Failed To Open The AVI Stream");
		return;
	}

	// Uncomment this when finished TODO 2

	AVIStreamInfo(pavi, &psi, sizeof(psi));					// Reads Information About The Stream Into psi
	width = psi.rcFrame.right - psi.rcFrame.left;           // Width Is Right Side Of Frame Minus Left
	height = psi.rcFrame.bottom - psi.rcFrame.top;          // Height Is Bottom Of Frame Minus Top
	lastFrame = AVIStreamLength(pavi);						// The Last Frame Of The Stream

															// TODO 3.1: Use AVIStreamFrameOpen(...) to decompress video frames from the AVI file. On the second parameter you can pass AVIGETFRAMEF_BESTDISPLAYFMT to select the best display format.
															// Hint: this function returns a PGETFRAME
	pgf = AVIStreamGetFrameOpen(pavi, (LPBITMAPINFOHEADER)AVIGETFRAMEF_BESTDISPLAYFMT);              // Create The PGETFRAME Using Our Request Mode
	if (pgf == NULL)
		LOG("Failed To Open The AVI Frame");

	isVideoFinished = false;
	videohasplayed = true;
	videojustended = true;
}

bool j1Video::PostUpdate()
{
	if (!App->video->isVideoFinished)
	{
		App->video->GrabAVIFrame();
		if (App->input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_BACKSPACE) == KEY_DOWN || App->input->GetMouseButtonDown(1) == KEY_DOWN)
		{
			isVideoFinished = true;
		}
	}
	else
	{
		if (videojustended)
		{
			LOG("video finished");
			App->uiscene->toggleMenu(true, START_MENU);
			App->audio->PlayMusic(MAIN_THEME);
			videojustended = false;
			SDL_ShowCursor(SDL_ENABLE);
			SDL_WarpMouseInWindow(App->win->window, App->win->width / 2 , App->win->height / 2 );
		}
	}
	return true;
}

bool j1Video::GrabAVIFrame()
{
	//Uncomment this after you have finished TODO 3.
	LPBITMAPINFOHEADER lpbi;													 // Holds The Bitmap Header Information
	lpbi = (LPBITMAPINFOHEADER)AVIStreamGetFrame(pgf, frame);					// Grab Data From The AVI Stream
	pdata = (char *)lpbi + lpbi->biSize + lpbi->biClrUsed * sizeof(RGBQUAD);    // Pointer To Data Returned By AVIStreamGetFrame
																				// (Skip The Header Info To Get To The Data)
																				// Convert Data To Requested Bitmap Format
	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(pdata, width, height, lpbi->biBitCount, width * 3, 0, 0, 0, 0);
	SDL_Texture* texture = App->tex->LoadSurface(surface);

	App->render->DrawQuad({0,0,App->win->width,App->win->height},Black,true,false);
	App->render->Blit(texture,1680/2-width/2 , 1050/2-height/2 , NULL, false,true,1.0f, SDL_FLIP_VERTICAL);

	if (i % 2 == 0)
	{
		frame++;
	}
	i++;
	if (frame >= lastFrame)
	{
		frame = 0;
		isVideoFinished = true;
	}

	App->tex->UnLoad(texture);
	SDL_FreeSurface(surface);


	mpf = AVIStreamSampleToTime(pavi, lastFrame) / lastFrame;        // Calculate Rough Milliseconds Per Frame

	return true;
}

void j1Video::CloseAVI()
{
	if (videohasplayed)
	{
		AVIStreamGetFrameClose(pgf);                // Deallocates The GetFrame Resources

		AVIStreamRelease(pavi);                     // Release The Stream

		AVIFileExit();                              // Release The File
	}
}
