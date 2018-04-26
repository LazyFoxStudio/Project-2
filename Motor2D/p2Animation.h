#ifndef __P2ANIMATION_H__
#define __P2ANIMATION_H__
#include <assert.h>
#include "SDL/include/SDL_rect.h"
#include "PugiXml\src\pugixml.hpp"

#define MAX_FRAMES 50


enum animationType
{
	IDLE_N, IDLE_NE, IDLE_E, IDLE_SE, IDLE_S, IDLE_SW, IDLE_W, IDLE_NW,
	MOVE_N, MOVE_NE, MOVE_E, MOVE_SE, MOVE_S, MOVE_SW, MOVE_W, MOVE_NW,
	ATK_N, ATK_NE, ATK_E, ATK_SE, ATK_S, ATK_SW, ATK_W, ATK_NW,
	DEAD_NE, DEAD_SE, DEAD_SW, DEAD_NW,
};

class Animation
{
public:
	bool loop = true;
	float speed = 0.05f;
	SDL_Rect frames[MAX_FRAMES];

private:

	float current_frame;
	int last_frame = 0;
	int loops = 0;

public:
	Animation() {};
	Animation(Animation& anim);
	bool LoadAnimation(pugi::xml_node& data, int width, int height);

	void PushBack(const SDL_Rect& rect);
	SDL_Rect& GetCurrentFrame(float dt);

	bool Finished() const;
	bool justFinished() const;
	void Reset();

	float GetCurrentFrameinFloat();
	int GetLastFrameinInt();
	void ChangeCurrentFrame(float frame);
};

#endif