#include "p2Log.h"
#include "j1App.h"
#include "j1Window.h"
#include "j1Textures.h"
#include "j1Render.h"

#include "j1Video.h"

j1Video::j1Video()
{
	name = "video";
}

j1Video::~j1Video()
{
}

bool j1Video::Awake(pugi::xml_node & node)
{
	AVIFileInit();                          // Opens The AVIFile Library

	folder = node.child("folder").attribute("value").as_string();
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

	if (AVIStreamOpenFromFile(&pavi, path, streamtypeVIDEO, 0, OF_READ, NULL) != 0) // Opens The AVI Stream
		LOG("Failed To Open The AVI Stream");

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
}

bool j1Video::PostUpdate()
{
	if (!App->video->isVideoFinished)
	{
		App->video->GrabAVIFrame();
	}
	else
	{
		LOG("video finished");
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

	App->render->Blit(texture, 0, 0, NULL, false,true,1.0f, SDL_FLIP_VERTICAL);

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
	AVIStreamGetFrameClose(pgf);                // Deallocates The GetFrame Resources

	AVIStreamRelease(pavi);                     // Release The Stream

	AVIFileExit();                              // Release The File
}
