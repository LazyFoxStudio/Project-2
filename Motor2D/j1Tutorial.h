#ifndef __J1TUTORIAL_H__
#define __J1TUTORIAL_H__

#include "j1Module.h"
#include "p2Point.h"
#include "j1Timer.h"
#include <list>

#define ARROW_MOVEMENT 50
#define ARROW_SPEED 8

class TextBox;
class Image;

struct ArrowInfo
{
	float rotation = 0;
	iPoint pointAt = { 0,0 };
};

struct Step
{
	int duration = 0;
	TextBox* text = nullptr;
	ArrowInfo* arrowInfo = nullptr;

	void Draw();
	bool isFinished();
};

class j1Tutorial : public j1Module
{
public:
	j1Tutorial();
	~j1Tutorial();

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool Update(float dt);

	void loadTutorial(char* path);
	void startTutorial();
	void finishStep();

public:

	Image* arrow = nullptr;
	Step* activeStep = nullptr;
	std::list<Step*> missing_steps;
	std::list<Step*> steps;
	bool doingTutorial = false;

	j1Timer timer;
};

#endif //__J1TUTORIAL_H__