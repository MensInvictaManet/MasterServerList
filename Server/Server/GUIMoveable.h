#pragma once

#include "GUIObjectNode.h"
#include "InputManager.h"

class GUIMoveable : public GUIObjectNode
{
public:
	static GUIMoveable* CreateMoveable(const char* imageFile, int x = 0, int y = 0, int w = 0, int h = 0, int grab_x = 0, int grab_y = 0, int grab_w = 0, int grab_h = 0);

	GUIMoveable(int grab_x, int grab_y, int grab_w, int grab_h);
	virtual ~GUIMoveable();

	void Input(int xOffset = 0, int yOffset = 0) override;
	void Render(int xOffset = 0, int yOffset = 0) override;

private:
	bool m_Grabbed;
	int m_GrabX;
	int m_GrabY;
	int m_GrabW;
	int m_GrabH;
	int m_GrabLastX;
	int m_GrabLastY;
};

inline GUIMoveable* GUIMoveable::CreateMoveable(const char* imageFile, int x, int y, int w, int h, int grab_x, int grab_y, int grab_w, int grab_h)
{
	auto newMoveable = new GUIMoveable(grab_x, grab_y, grab_w, grab_h);
	newMoveable->SetTextureID(textureManager.LoadTextureGetID(imageFile));
	newMoveable->SetX(x);
	newMoveable->SetY(y);
	newMoveable->SetWidth(w);
	newMoveable->SetHeight(h);
	return newMoveable;
}

inline GUIMoveable::GUIMoveable(int grab_x, int grab_y, int grab_w, int grab_h) :
	m_Grabbed(false),
	m_GrabX(grab_x),
	m_GrabY(grab_y),
	m_GrabW(grab_w),
	m_GrabH(grab_h),
	m_GrabLastX(0),
	m_GrabLastY(0)
{
	
}

inline GUIMoveable::~GUIMoveable()
{
	
}

inline void GUIMoveable::Input(int xOffset, int yOffset)
{
	auto leftButtonState = inputManager.GetMouseButtonLeft();
	auto x = inputManager.GetMouseX();
	auto y = inputManager.GetMouseY();

	if (leftButtonState == MOUSE_BUTTON_UNPRESSED) m_Grabbed = false;
	else if (m_Grabbed == false)
	{
		if ((x > xOffset + m_X + m_GrabX) && (x < xOffset + m_X + m_GrabX + m_GrabW) && (y > yOffset + m_Y + m_GrabY) && (y < yOffset + m_Y + m_GrabY + m_GrabH))
		{
			if (leftButtonState == MOUSE_BUTTON_PRESSED)
			{
				m_Grabbed = true;
				m_GrabLastX = x;
				m_GrabLastY = y;
			}
		}
	}
	else
	{
		m_X += (x - m_GrabLastX);
		m_Y += (y - m_GrabLastY);
		m_GrabLastX = x;
		m_GrabLastY = y;
	}

	//  Take base node input
	GUIObjectNode::Input(xOffset, yOffset);
}

inline void GUIMoveable::Render(int xOffset, int yOffset)
{
	//  Do the base node render
	GUIObjectNode::Render(xOffset, yOffset);
}