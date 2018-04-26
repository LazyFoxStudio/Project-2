#include "p2Animation.h"

Animation::Animation(Animation& anim)
{
	speed = anim.speed;
	loop = anim.loop;

	for (int i = 0; i < anim.last_frame; i++)
	{
		PushBack(anim.frames[i]);
	}
	//for (int i = anim.last_frame-2; i >= 0; i--)
	//{
	//	PushBack(anim.frames[i]);
	//}
}

void Animation::PushBack(const SDL_Rect& rect)
{
	if (last_frame < MAX_FRAMES)
		frames[last_frame++] = rect;
	assert(last_frame < MAX_FRAMES);
}

SDL_Rect& Animation::GetCurrentFrame(float dt)
{
	float tmp = speed;
	current_frame += tmp * dt;

	if (current_frame < 0)
	{
		current_frame = 0;
	}

	if (current_frame >= last_frame)
	{
		current_frame = (loop) ? 0.0f : last_frame - 1;
		loops++;
	}

	return frames[(int)current_frame];
}

bool Animation::LoadAnimation(pugi::xml_node& data, int width, int height)
{
	int rows = data.child("rows").attribute("value").as_int(0);
	int columns = data.child("columns").attribute("value").as_int(0);
	int StartingRow = data.child("StartingRow").attribute("value").as_int(0);
	int StartingColumn = data.child("StartingColumn").attribute("value").as_int(0);
	speed = data.child("speed").attribute("value").as_float(10.0f);
	loop = data.child("loop").attribute("value").as_bool(true);

	for (int i = StartingRow; i <= StartingRow + rows; i++) 
		for (int j = StartingColumn; j <= StartingColumn + columns; j++) 
			PushBack({ width*j,height*i,width,height });

	return true;
}

bool Animation::Finished() const				{ return loops > 0; }
bool Animation::justFinished() const			
{ 
	if ((int)current_frame == last_frame - 2)
		return true;
	else
		return false;
	
}
void Animation::Reset()							{ current_frame = 0; }
float Animation::GetCurrentFrameinFloat()		{ return current_frame; }
int Animation::GetLastFrameinInt()				{ return last_frame;}
void Animation::ChangeCurrentFrame(float frame)	{ current_frame = frame; }