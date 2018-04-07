#include "UI_element.h"
#include "j1Render.h"

UI_element::~UI_element()
{
	if (texture != nullptr && texture != App->gui->GetAtlas() && texture != App->gui->GetIconAtlas())
	{
		App->tex->UnLoad(texture);
		texture = nullptr;
	}

	RELEASE(popUp);
}

iPoint UI_element::calculateAbsolutePosition() const
{
	if (parent != nullptr)
		return (localPosition + parent->calculateAbsolutePosition());
	else
		return localPosition;
}

void UI_element::appendChild(UI_element * child, bool center)
{
	if (center)
	{
		iPoint child_pos(section.w / 2, section.h / 2);
		child_pos.x -= child->section.w / 2;
		child_pos.y -= child->section.h / 2;
		child->localPosition = child_pos;
		child->setOriginalPos(child_pos.x, child_pos.y);
	}	
	child->parent = this;
	childs.push_back(child);
}

void UI_element::BlitChilds()
{
	for (std::list<UI_element*>::iterator it_e = childs.begin(); it_e != childs.end(); it_e++)
	{
		(*it_e)->BlitElement();
	}
}

void UI_element::setDragable(bool horizontally, bool vertically)
{
	if (horizontally || vertically)
		dragable = true;
	else
		dragable = false;

	horizontalMovement = horizontally;
	verticalMovement = vertically;
}

void UI_element::setOriginalPos(int x, int y)
{
	Original_Pos = { x, y };
}

void UI_element::setLimits(int right_limit, int left_limit, int top_limit, int bottom_limit)
{
	this->right_limit = right_limit;
	this->left_limit = left_limit;
	this->top_limit = top_limit;
	this->bottom_limit = bottom_limit;
}

void UI_element::Mouse_Drag()
{
	iPoint Mouse_Movement;
	App->input->GetMousePosition(Mouse_Movement.x, Mouse_Movement.y);
	if (horizontalMovement)
	{
		localPosition.x += (Mouse_Movement.x - Click_Pos.x);
		Click_Pos.x += (Mouse_Movement.x - Click_Pos.x);
	}
	if (verticalMovement)
	{
		localPosition.y += (Mouse_Movement.y - Click_Pos.y);
		Click_Pos.y += (Mouse_Movement.y - Click_Pos.y);
	}

	if (left_limit >= 0 && localPosition.x < Original_Pos.x - left_limit)
		localPosition.x = Original_Pos.x - left_limit;
	else if (right_limit >= 0 && localPosition.x > Original_Pos.x + right_limit)
		localPosition.x = Original_Pos.x + right_limit;
	if (top_limit >= 0 && localPosition.y < Original_Pos.y - top_limit)
		localPosition.y = Original_Pos.y - top_limit;
	else if (bottom_limit >= 0 && localPosition.y > Original_Pos.y + bottom_limit)
		localPosition.y = Original_Pos.y + bottom_limit;
}

void UI_element::Start_Drag()
{
	iPoint Mouse_Movement;
	App->input->GetMousePosition(Mouse_Movement.x, Mouse_Movement.y);
	Click_Pos = Mouse_Movement;

	moving = true;
}

void UI_element::End_Drag()
{
	Click_Pos = { 0,0 };
	moving = false;
}