#ifndef __P2ANIMATION_H__
#define __P2ANIMATION_H__
#include <assert.h>
#include "SDL/include/SDL_rect.h"
#include "PugiXml\src\pugixml.hpp"

#define MAX_FRAMES 50


enum animationType
{
	IDLE_N, IDLENE, IDLE_E, IDLESE, IDLE_S,
	MOVE_N, MOVE_NE, MOVE_E, MOVE_SE, MOVE_S,
	ATK_E, ATK_S, ATK_W, ATK_N,
	DEAD
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
	bool LoadAnimation(pugi::xml_node& data);

	void PushBack(const SDL_Rect& rect);
	SDL_Rect& GetCurrentFrame(float dt);

	bool Finished() const;
	void Reset();

	float GetCurrentFrameinFloat();
	int GetLastFrameinInt();
	void ChangeCurrentFrame(float frame);
};

#endif