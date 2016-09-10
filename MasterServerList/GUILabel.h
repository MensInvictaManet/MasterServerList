#pragma once

#include "GUIObjectNode.h"
#include "FontManager.h"

class GUILabel : public GUIObjectNode
{
public:
	static GUILabel* CreateLabel(const char* text, int x = 0, int y = 0, int w = 0, int h = 0);

	explicit GUILabel(const char* text);
	virtual ~GUILabel();

	void SetFont(const Font* font) { m_Font = font; }
	void SetText(const std::string text) { m_Text = text; }

	void Input(int xOffset = 0, int yOffset = 0) override {};
	void Render(int xOffset = 0, int yOffset = 0) override;

private:
	const Font* m_Font;
	std::string m_Text;
};

inline GUILabel* GUILabel::CreateLabel(const char* text, int x, int y, int w, int h)
{
	auto newLabel = new GUILabel(text);
	newLabel->SetX(x);
	newLabel->SetY(y);
	newLabel->SetWidth(w);
	newLabel->SetHeight(h);
	return newLabel;
}

inline GUILabel::GUILabel(const char* text) :
	m_Font(nullptr),
	m_Text("")
{

}


inline GUILabel::~GUILabel()
{
	
}

inline void GUILabel::Render(int xOffset, int yOffset)
{
	//  Render the object if we're able
	if (!m_SetToDestroy && m_Visible && (m_Font != nullptr && !m_Text.empty()) && m_Width > 0 && m_Height > 0)
	{
		auto x = m_X + xOffset;
		auto y = m_Y + yOffset;

		//  Render the font the same way regardless of templating
		m_Font->RenderText(m_Text.c_str(), x + m_Width / 2, y + m_Height / 2, true, true);
	}

	//  Pass the render call to all children
	for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter) (*iter)->Render(xOffset + m_X, yOffset + m_Y);
}