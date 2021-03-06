#pragma once

#include "GUIObjectNode.h"
#include "InputManager.h"
#include "FontManager.h"
#include "TimeSlice.h"

const float TIME_BETWEEN_BACKSPACES = 0.1f;

class GUIEditBox : public GUIObjectNode
{
public:
	static GUIEditBox* CreateEditBox(const char* imageFile, int x = 0, int y = 0, int w = 0, int h = 0);
	static GUIEditBox* CreateTemplatedEditBox(const char* editboxTemplate, int x = 0, int y = 0, int w = 0, int h = 0);

	explicit GUIEditBox(bool templated);
	virtual ~GUIEditBox();

	inline void SetFont(const Font* font) { m_Font = font; }
	inline void SetText(const std::string text) { m_Text = text; }
	inline std::string GetText() const { return m_Text; }

	void Input(int xOffset = 0, int yOffset = 0) override;
	void Render(int xOffset = 0, int yOffset = 0) override;

private:
	bool m_Selected;
	const Font* m_Font;
	std::string m_Text;
	float m_LastBackspaceTime;

	bool m_Templated;
	TextureManager::ManagedTexture* TextureTopLeftCorner[2];
	TextureManager::ManagedTexture* TextureTopRightCorner[2];
	TextureManager::ManagedTexture* TextureBottomLeftCorner[2];
	TextureManager::ManagedTexture* TextureBottomRightCorner[2];
	TextureManager::ManagedTexture* TextureLeftSide[2];
	TextureManager::ManagedTexture* TextureRightSide[2];
	TextureManager::ManagedTexture* TextureTopSide[2];
	TextureManager::ManagedTexture* TextureBottomSide[2];
	TextureManager::ManagedTexture* TextureMiddle[2];
};

inline GUIEditBox* GUIEditBox::CreateEditBox(const char* imageFile, int x, int y, int w, int h)
{
	auto newEditbox = new GUIEditBox(false);
	newEditbox->SetTextureID(textureManager.LoadTextureGetID(imageFile));
	newEditbox->SetX(x);
	newEditbox->SetY(y);
	newEditbox->SetWidth(w);
	newEditbox->SetHeight(h);
	return newEditbox;
}

inline GUIEditBox* GUIEditBox::CreateTemplatedEditBox(const char* editboxTemplate, int x, int y, int w, int h)
{
	auto newEditbox = new GUIEditBox(true);

	auto templateFolder("./UITemplates/EditBox/" + std::string(editboxTemplate) + "/");
	newEditbox->TextureTopLeftCorner[0] = textureManager.LoadTexture(std::string(templateFolder + "U_TopLeftCorner.png").c_str());
	newEditbox->TextureTopRightCorner[0] = textureManager.LoadTexture(std::string(templateFolder + "U_TopRightCorner.png").c_str());
	newEditbox->TextureBottomLeftCorner[0] = textureManager.LoadTexture(std::string(templateFolder + "U_BottomLeftCorner.png").c_str());
	newEditbox->TextureBottomRightCorner[0] = textureManager.LoadTexture(std::string(templateFolder + "U_BottomRightCorner.png").c_str());
	newEditbox->TextureLeftSide[0] = textureManager.LoadTexture(std::string(templateFolder + "U_LeftSide.png").c_str());
	newEditbox->TextureRightSide[0] = textureManager.LoadTexture(std::string(templateFolder + "U_RightSide.png").c_str());
	newEditbox->TextureTopSide[0] = textureManager.LoadTexture(std::string(templateFolder + "U_TopSide.png").c_str());
	newEditbox->TextureBottomSide[0] = textureManager.LoadTexture(std::string(templateFolder + "U_BottomSide.png").c_str());
	newEditbox->TextureMiddle[0] = textureManager.LoadTexture(std::string(templateFolder + "U_Middle.png").c_str());
	newEditbox->TextureTopLeftCorner[1] = textureManager.LoadTexture(std::string(templateFolder + "C_TopLeftCorner.png").c_str());
	newEditbox->TextureTopRightCorner[1] = textureManager.LoadTexture(std::string(templateFolder + "C_TopRightCorner.png").c_str());
	newEditbox->TextureBottomLeftCorner[1] = textureManager.LoadTexture(std::string(templateFolder + "C_BottomLeftCorner.png").c_str());
	newEditbox->TextureBottomRightCorner[1] = textureManager.LoadTexture(std::string(templateFolder + "C_BottomRightCorner.png").c_str());
	newEditbox->TextureLeftSide[1] = textureManager.LoadTexture(std::string(templateFolder + "C_LeftSide.png").c_str());
	newEditbox->TextureRightSide[1] = textureManager.LoadTexture(std::string(templateFolder + "C_RightSide.png").c_str());
	newEditbox->TextureTopSide[1] = textureManager.LoadTexture(std::string(templateFolder + "C_TopSide.png").c_str());
	newEditbox->TextureBottomSide[1] = textureManager.LoadTexture(std::string(templateFolder + "C_BottomSide.png").c_str());
	newEditbox->TextureMiddle[1] = textureManager.LoadTexture(std::string(templateFolder + "C_Middle.png").c_str());
	newEditbox->SetTextureID(0);

	newEditbox->SetX(x);
	newEditbox->SetY(y);
	newEditbox->SetWidth(w);
	newEditbox->SetHeight(h);
	return newEditbox;
}

inline GUIEditBox::GUIEditBox(bool templated) :
	m_Selected(false),
	m_Font(nullptr),
	m_Text(""),
	m_Templated(templated)
{
	TextureTopLeftCorner[0] = TextureTopLeftCorner[1] = nullptr;
	TextureTopRightCorner[0] = TextureTopRightCorner[1] = nullptr;
	TextureBottomLeftCorner[0] = TextureBottomLeftCorner[1] = nullptr;
	TextureBottomRightCorner[0] = TextureBottomRightCorner[1] = nullptr;
	TextureLeftSide[0] = TextureLeftSide[1] = nullptr;
	TextureRightSide[0] = TextureRightSide[1] = nullptr;
	TextureTopSide[0] = TextureTopSide[1] = nullptr;
	TextureBottomSide[0] = TextureBottomSide[1] = nullptr;
	TextureMiddle[0] = TextureMiddle[1] = nullptr;
}


inline GUIEditBox::~GUIEditBox()
{
	
}


inline void GUIEditBox::Input(int xOffset, int yOffset)
{
	if (m_SetToDestroy || !m_Visible) return;

	auto leftButtonState = inputManager.GetMouseButtonLeft();
	auto x = inputManager.GetMouseX();
	auto y = inputManager.GetMouseY();

	//  If selected, take keyboard text input
	if (m_Selected)
	{
		if (inputManager.GetBackspace() && !m_Text.empty() && (gameSeconds - m_LastBackspaceTime >= TIME_BETWEEN_BACKSPACES))
		{
			m_Text.erase(--m_Text.end());
			m_LastBackspaceTime = gameSeconds;
		}
		else m_Text += inputManager.GetKeyboardString();
	}

	if (leftButtonState == MOUSE_BUTTON_PRESSED && ((x > xOffset + m_X) && (x < xOffset + m_X + m_Width) && (y > yOffset + m_Y) && (y < yOffset + m_Y + m_Height)))
	{
		inputManager.TakeMouseButtonLeft();
		m_Selected = true;
	}

	//  Take base node input
	GUIObjectNode::Input(xOffset, yOffset);
}

inline void GUIEditBox::Render(int xOffset, int yOffset)
{
	//  Render the object if we're able
	if (!m_SetToDestroy && m_Visible && ((m_TextureID != 0) || m_Templated) && m_Width > 0 && m_Height > 0)
	{
		auto x = m_X + xOffset;
		auto y = m_Y + yOffset;

		if (m_Templated)
		{
			auto pressedIndex = (m_Selected ? 1 : 0);

			TextureTopLeftCorner[pressedIndex]->RenderTexture(x, y, TextureTopLeftCorner[pressedIndex]->getWidth(), TextureTopLeftCorner[pressedIndex]->getHeight());
			TextureTopRightCorner[pressedIndex]->RenderTexture(x + m_Width - TextureTopRightCorner[pressedIndex]->getWidth(), y, TextureTopRightCorner[pressedIndex]->getWidth(), TextureTopRightCorner[pressedIndex]->getHeight());
			TextureBottomLeftCorner[pressedIndex]->RenderTexture(x, y + m_Height - TextureBottomLeftCorner[pressedIndex]->getHeight(), TextureBottomLeftCorner[pressedIndex]->getWidth(), TextureBottomLeftCorner[pressedIndex]->getHeight());
			TextureBottomRightCorner[pressedIndex]->RenderTexture(x + m_Width - TextureBottomRightCorner[pressedIndex]->getWidth(), y + m_Height - TextureBottomRightCorner[pressedIndex]->getHeight(), TextureBottomRightCorner[pressedIndex]->getWidth(), TextureBottomLeftCorner[pressedIndex]->getHeight());
			TextureLeftSide[pressedIndex]->RenderTexture(x, y + TextureTopLeftCorner[pressedIndex]->getHeight(), TextureLeftSide[pressedIndex]->getWidth(), m_Height - TextureTopLeftCorner[pressedIndex]->getHeight() - TextureBottomLeftCorner[pressedIndex]->getHeight());
			TextureRightSide[pressedIndex]->RenderTexture(x + m_Width - TextureRightSide[pressedIndex]->getWidth(), y + TextureTopRightCorner[pressedIndex]->getHeight(), TextureRightSide[pressedIndex]->getWidth(), m_Height - TextureTopRightCorner[pressedIndex]->getHeight() - TextureBottomRightCorner[pressedIndex]->getHeight());
			TextureTopSide[pressedIndex]->RenderTexture(x + TextureTopLeftCorner[pressedIndex]->getWidth(), y, m_Width - TextureBottomLeftCorner[pressedIndex]->getWidth() - TextureBottomRightCorner[pressedIndex]->getWidth(), TextureTopSide[pressedIndex]->getHeight());
			TextureBottomSide[pressedIndex]->RenderTexture(x + TextureBottomLeftCorner[pressedIndex]->getWidth(), y + m_Height - TextureBottomSide[pressedIndex]->getHeight(), m_Width - TextureBottomLeftCorner[pressedIndex]->getWidth() - TextureBottomRightCorner[pressedIndex]->getWidth(), TextureBottomSide[pressedIndex]->getHeight());
			TextureMiddle[pressedIndex]->RenderTexture(x + TextureLeftSide[pressedIndex]->getWidth(), y + TextureTopSide[pressedIndex]->getHeight(), m_Width - TextureLeftSide[pressedIndex]->getWidth() - TextureRightSide[pressedIndex]->getWidth(), m_Height - TextureTopSide[pressedIndex]->getHeight() - TextureBottomSide[pressedIndex]->getHeight());
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, m_TextureID);

			auto pressedWidthDelta = m_Selected ? int(m_Width * 0.05f) : 0;
			auto pressedHeightDelta = m_Selected ? int(m_Height * 0.05f) : 0;

			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f); glVertex2i(x + pressedWidthDelta, y + pressedHeightDelta);
			glTexCoord2f(1.0f, 0.0f); glVertex2i(x + m_Width - pressedWidthDelta, y + pressedHeightDelta);
			glTexCoord2f(1.0f, 1.0f); glVertex2i(x + m_Width - pressedWidthDelta, y + m_Height - pressedHeightDelta);
			glTexCoord2f(0.0f, 1.0f); glVertex2i(x + pressedWidthDelta, y + m_Height - pressedHeightDelta);
			glEnd();
		}

		//  Render the font the same way regardless of templating
		if (m_Font != nullptr && !m_Text.empty())
		{
			m_Font->RenderText(m_Text.c_str(), x + m_Width / 2, y + m_Height / 2, true, true);
		}
	}

	//  Pass the render call to all children
	for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter) (*iter)->Render(xOffset + m_X, yOffset + m_Y);
}