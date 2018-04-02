#ifndef __P2ANIMATION_H__
#define __P2ANIMATION_H__
#include <assert.h>
#include "SDL/include/SDL_rect.h"
#include "PugiXml\src\pugixml.hpp"

#define MAX_FRAMES 50


enum animationType
{
	IDLEN, IDLENE, IDLEE, IDLESE, IDLES,
	MOV_N, MOVE_NE, MOV_E, MOV_SE, MOV_S,
	ATK_E, ATK_S, ATK_W, ATK_N,
	DEAD
};

class Animation
{
public:
	bool loop = true;
	float speed = 1.0f;
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