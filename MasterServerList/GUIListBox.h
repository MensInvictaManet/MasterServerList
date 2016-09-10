#pragma once

#include "GUIObjectNode.h"
#include "InputManager.h"
#include "TimeSlice.h"

#include <functional>

#define LIST_BOX_SCROLL_DELAY 0.1f

class GUIListBox : public GUIObjectNode
{
public:
	enum Justifications { JUSTIFY_LEFT = 0, JUSTIFY_RIGHT, JUSTIFY_CENTER, JUSTIFICATION_COUNT };
	typedef std::function<void(GUIObjectNode*)> GUIListBoxCallback;

	static GUIListBox* CreateListBox(const char* imageFile, int x = 0, int y = 0, int w = 0, int h = 0);
	static GUIListBox* CreateTemplatedListBox(const char* listboxTemplate, int x = 0, int y = 0, int w = 0, int h = 0, int dirButtonsX = 0, int contentY = 0, int upButtonW = 0, int upButtonH = 0, int DownButtonW = 0, int downButtonH = 0, int barColumnW = 0, int entryHeight = 0, int spaceBetweenEntries = 0);

	explicit GUIListBox(bool templated);
	virtual ~GUIListBox();

	void SetItemClickCallback(const GUIListBoxCallback& callback) { m_ItemClickCallback = callback; }
	void Input(int xOffset = 0, int yOffset = 0) override;
	void Render(int xOffset = 0, int yOffset = 0) override;

	inline void AddItem(GUIObjectNode* item) { m_ItemList.push_back(item); UpdateMover(); }

private:
	GUIListBoxCallback	m_ItemClickCallback;

	std::vector<GUIObjectNode*> m_ItemList;
	int	SelectedIndex;
	int MovementIndex;
	int MoverHeight;
	int MoverY;
	int MoverYDelta;
	bool m_Clicked;
	int ClickedY;
	float m_LastClickTime;

	bool m_Templated;
	TextureManager::ManagedTexture* TextureTopLeftCorner;
	TextureManager::ManagedTexture* TextureTopRightCorner;
	TextureManager::ManagedTexture* TextureBottomLeftCorner;
	TextureManager::ManagedTexture* TextureBottomRightCorner;
	TextureManager::ManagedTexture* TextureLeftSide;
	TextureManager::ManagedTexture* TextureRightSide;
	TextureManager::ManagedTexture* TextureTopSide;
	TextureManager::ManagedTexture* TextureBottomSide;
	TextureManager::ManagedTexture* TextureMiddle;
	TextureManager::ManagedTexture* TextureUpButton;
	TextureManager::ManagedTexture* TextureDownButton;
	TextureManager::ManagedTexture* TextureBarColumn;
	TextureManager::ManagedTexture* TextureMoverTop;
	TextureManager::ManagedTexture* TextureMoverMiddle;
	TextureManager::ManagedTexture* TextureMoverBottom;
	TextureManager::ManagedTexture* TextureSelector;
	int DirectionalButtonsX;
	int ContentY;
	int UpButtonW;
	int UpButtonH;
	int DownButtonW;
	int DownButtonH;
	int BarColumnW;
	int EntryHeight;
	int SpaceBetweenEntries;
	int ItemDisplayCount;
	int Justification;

	void UpdateMover(void);
};

inline GUIListBox* GUIListBox::CreateListBox(const char* imageFile, int x, int y, int w, int h)
{
	auto newListbox = new GUIListBox(false);
	newListbox->SetTextureID(textureManager.LoadTextureGetID(imageFile));
	newListbox->SetX(x);
	newListbox->SetY(y);
	newListbox->SetWidth(w);
	newListbox->SetHeight(h);
	return newListbox;
}

inline GUIListBox* GUIListBox::CreateTemplatedListBox(const char* listboxTemplate, int x, int y, int w, int h, int dirButtonsX, int contentY, int upButtonW, int upButtonH, int DownButtonW, int downButtonH, int barColumnW, int entryHeight, int spaceBetweenEntries)
{
	auto newListbox = new GUIListBox(true);

	auto templateFolder("./UITemplates/ListBox/" + std::string(listboxTemplate) + "/");
	newListbox->TextureTopLeftCorner = textureManager.LoadTexture(std::string(templateFolder + "TopLeftCorner.png").c_str());
	newListbox->TextureTopRightCorner = textureManager.LoadTexture(std::string(templateFolder + "TopRightCorner.png").c_str());
	newListbox->TextureBottomLeftCorner = textureManager.LoadTexture(std::string(templateFolder + "BottomLeftCorner.png").c_str());
	newListbox->TextureBottomRightCorner = textureManager.LoadTexture(std::string(templateFolder + "BottomRightCorner.png").c_str());
	newListbox->TextureLeftSide = textureManager.LoadTexture(std::string(templateFolder + "LeftSide.png").c_str());
	newListbox->TextureRightSide = textureManager.LoadTexture(std::string(templateFolder + "RightSide.png").c_str());
	newListbox->TextureTopSide = textureManager.LoadTexture(std::string(templateFolder + "TopSide.png").c_str());
	newListbox->TextureBottomSide = textureManager.LoadTexture(std::string(templateFolder + "BottomSide.png").c_str());
	newListbox->TextureMiddle = textureManager.LoadTexture(std::string(templateFolder + "Middle.png").c_str());
	newListbox->TextureUpButton = textureManager.LoadTexture(std::string(templateFolder + "UpButton.png").c_str());
	newListbox->TextureDownButton = textureManager.LoadTexture(std::string(templateFolder + "DownButton.png").c_str());
	newListbox->TextureBarColumn = textureManager.LoadTexture(std::string(templateFolder + "BarColumn.png").c_str());
	newListbox->TextureMoverTop = textureManager.LoadTexture(std::string(templateFolder + "MoverTop.png").c_str());
	newListbox->TextureMoverMiddle = textureManager.LoadTexture(std::string(templateFolder + "MoverMiddle.png").c_str());
	newListbox->TextureMoverBottom = textureManager.LoadTexture(std::string(templateFolder + "MoverBottom.png").c_str());
	newListbox->TextureSelector = textureManager.LoadTexture(std::string(templateFolder + "Selector.png").c_str());
	newListbox->SetTextureID(0);

	newListbox->DirectionalButtonsX = dirButtonsX;
	newListbox->ContentY = contentY;
	newListbox->UpButtonW = upButtonW;
	newListbox->UpButtonH = upButtonH;
	newListbox->DownButtonW = DownButtonW;
	newListbox->DownButtonH = downButtonH;
	newListbox->BarColumnW = barColumnW;
	newListbox->EntryHeight = entryHeight;
	newListbox->SpaceBetweenEntries = spaceBetweenEntries;

	newListbox->SetX(x);
	newListbox->SetY(y);
	newListbox->SetWidth(w);
	newListbox->SetHeight(h);

	newListbox->ItemDisplayCount = h / (entryHeight + spaceBetweenEntries);

	return newListbox;
}

inline GUIListBox::GUIListBox(bool templated) :
	m_ItemClickCallback(nullptr),
	SelectedIndex(-1),
	MovementIndex(0),
	MoverHeight(-1),
	MoverY(-1),
	MoverYDelta(-1),
	m_Clicked(false),
	ClickedY(-1),
	m_LastClickTime(0.0f),
	m_Templated(templated)
{
	TextureTopLeftCorner = nullptr;
	TextureTopRightCorner = nullptr;
	TextureBottomLeftCorner = nullptr;
	TextureBottomRightCorner = nullptr;
	TextureLeftSide = nullptr;
	TextureRightSide = nullptr;
	TextureTopSide = nullptr;
	TextureBottomSide = nullptr;
	TextureMiddle = nullptr;
	TextureUpButton = nullptr;
	TextureDownButton = nullptr;
	TextureBarColumn = nullptr;
	TextureMoverTop = nullptr;
	TextureMoverMiddle = nullptr;
	TextureMoverBottom = nullptr;
	TextureSelector = nullptr;
	DirectionalButtonsX = 0;
	ContentY = 0;
	UpButtonW = -1;
	UpButtonH = -1;
	DownButtonW = -1;
	DownButtonH = -1;
	BarColumnW = -1;
	EntryHeight = -1;
	SpaceBetweenEntries = -1;
	ItemDisplayCount = -1;
	Justification = JUSTIFICATION_COUNT;
}



inline GUIListBox::~GUIListBox()
{
	
}


inline void GUIListBox::Input(int xOffset, int yOffset)
{
	auto leftButtonState = inputManager.GetMouseButtonLeft();
	auto middleButtonState = inputManager.GetMouseButtonMiddle();
	auto rightButtonState = inputManager.GetMouseButtonRight();
	auto x = m_X + xOffset;
	auto y = m_Y + yOffset;
	auto mx = inputManager.GetMouseX();
	auto my = inputManager.GetMouseY();

	if (m_Clicked)
	{
		if (leftButtonState == MOUSE_BUTTON_UNPRESSED)
		{
			m_Clicked = false;
			return;
		}

		if (std::abs(my - ClickedY) > MoverYDelta)
		{
			//  The farthest we can shift is to the top or bottom of the list
			auto indexChange = std::min(std::max((my - ClickedY) / MoverYDelta, -MovementIndex), (int(m_ItemList.size()) - ItemDisplayCount - MovementIndex));
			ClickedY += indexChange * MoverYDelta;
			MovementIndex += indexChange;
			UpdateMover();
		}
	}

	if (leftButtonState == MOUSE_BUTTON_UNPRESSED) return;
	if (mx < x || mx > x + m_Width || my < y || my > y + m_Height) return;

	//  If we're left of the directional buttons, assume we're clicking an entry in the list and find out which one
	if (mx < x + DirectionalButtonsX)
	{
		int newSelectedIndex = (int(my) - y - TextureTopSide->getHeight()) / (EntryHeight + SpaceBetweenEntries) + MovementIndex;
		if (newSelectedIndex >= MovementIndex && newSelectedIndex < int(m_ItemList.size()) && newSelectedIndex < MovementIndex + int(ItemDisplayCount))
		{
			if (SelectedIndex != newSelectedIndex && m_ItemClickCallback != nullptr) m_ItemClickCallback(this);
			SelectedIndex = newSelectedIndex;
			return;
		}
	}

	//  If we're clicking to the right of where the directional buttons start horizontally...
	if ((gameSeconds - m_LastClickTime > LIST_BOX_SCROLL_DELAY) && mx > x + DirectionalButtonsX)
	{
		//  If we're clicking the up button, move the movement index up one if possible
		if ((mx < x + DirectionalButtonsX + UpButtonW) && (my > y + ContentY) && (my < y + ContentY + UpButtonH))
		{
			if (MovementIndex > 0) MovementIndex -= 1;
			UpdateMover();
			m_LastClickTime = gameSeconds;
			return;
		}

		//  If we're clicking the down button, move the movement index down one if possible
		if ((mx < x + DirectionalButtonsX + DownButtonW) && (my > y + m_Height - ContentY - DownButtonH) && (my < y + m_Height - ContentY))
		{
			if (MovementIndex < int(m_ItemList.size() - ItemDisplayCount)) MovementIndex += 1;
			UpdateMover();
			m_LastClickTime = gameSeconds;
			return;
		}
	}

	//  If we're clicking inside of the mover, keep track of our click so we can drag it
	if ((leftButtonState == MOUSE_BUTTON_PRESSED) && (mx > x + DirectionalButtonsX) && (mx < x + DirectionalButtonsX + BarColumnW) && (my > y + ContentY + UpButtonH + MoverY) && (my < y + ContentY + UpButtonH + MoverY + MoverHeight))
	{
		m_Clicked = true;
		ClickedY = my;
	}

	//  Take base node input
	GUIObjectNode::Input(xOffset, yOffset);
}

inline void GUIListBox::Render(int xOffset, int yOffset)
{
	auto x = m_X + xOffset;
	auto y = m_Y + yOffset;

	//  Render the object if we're able
	if (!m_SetToDestroy && m_Visible && ((m_TextureID != 0) || m_Templated) && m_Width > 0 && m_Height > 0)
	{
		//  Render the background object, templated or single-textured
		if (m_Templated)
		{
			TextureTopLeftCorner->RenderTexture(x, y, TextureTopLeftCorner->getWidth(), TextureTopLeftCorner->getHeight());
			TextureTopRightCorner->RenderTexture(x + m_Width - TextureTopRightCorner->getWidth(), y, TextureTopRightCorner->getWidth(), TextureTopRightCorner->getHeight());
			TextureBottomLeftCorner->RenderTexture(x, y + m_Height - TextureBottomLeftCorner->getHeight(), TextureBottomLeftCorner->getWidth(), TextureBottomLeftCorner->getHeight());
			TextureBottomRightCorner->RenderTexture(x + m_Width - TextureBottomRightCorner->getWidth(), y + m_Height - TextureBottomRightCorner->getHeight(), TextureBottomRightCorner->getWidth(), TextureBottomLeftCorner->getHeight());
			TextureLeftSide->RenderTexture(x, y + TextureTopLeftCorner->getHeight(), TextureLeftSide->getWidth(), m_Height - TextureTopLeftCorner->getHeight() - TextureBottomLeftCorner->getHeight());
			TextureRightSide->RenderTexture(x + m_Width - TextureRightSide->getWidth(), y + TextureTopRightCorner->getHeight(), TextureRightSide->getWidth(), m_Height - TextureTopRightCorner->getHeight() - TextureBottomRightCorner->getHeight());
			TextureTopSide->RenderTexture(x + TextureTopLeftCorner->getWidth(), y, m_Width - TextureBottomLeftCorner->getWidth() - TextureBottomRightCorner->getWidth(), TextureTopSide->getHeight());
			TextureBottomSide->RenderTexture(x + TextureBottomLeftCorner->getWidth(), y + m_Height - TextureBottomSide->getHeight(), m_Width - TextureBottomLeftCorner->getWidth() - TextureBottomRightCorner->getWidth(), TextureBottomSide->getHeight());
			TextureMiddle->RenderTexture(x + TextureLeftSide->getWidth(), y + TextureTopSide->getHeight(), m_Width - TextureLeftSide->getWidth() - TextureRightSide->getWidth(), m_Height - TextureTopSide->getHeight() - TextureBottomSide->getHeight());
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, m_TextureID);

			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f); glVertex2i(x, y);
			glTexCoord2f(1.0f, 0.0f); glVertex2i(x + m_Width, y);
			glTexCoord2f(1.0f, 1.0f); glVertex2i(x + m_Width, y + m_Height);
			glTexCoord2f(0.0f, 1.0f); glVertex2i(x, y + m_Height);
			glEnd();
		}

		//  Render the scroll buttons if needed
		auto renderScrollButtons = (int(m_ItemList.size()) > ItemDisplayCount);
		if (renderScrollButtons)
		{
			TextureUpButton->RenderTexture(x + DirectionalButtonsX, y + ContentY, UpButtonW, UpButtonH);
			TextureBarColumn->RenderTexture(x + DirectionalButtonsX, y + ContentY + UpButtonH, BarColumnW, (m_Height - ContentY - DownButtonH) - (ContentY + UpButtonH));
			TextureDownButton->RenderTexture(x + DirectionalButtonsX, y + m_Height - ContentY - DownButtonH, DownButtonW, DownButtonH);
			TextureMoverTop->RenderTexture(x + DirectionalButtonsX + 1, y + ContentY + UpButtonH + MoverY, BarColumnW - 2, TextureMoverTop->getHeight());
			TextureMoverMiddle->RenderTexture(x + DirectionalButtonsX + 1, y + ContentY + UpButtonH + MoverY + TextureMoverTop->getHeight(), BarColumnW - 2, MoverHeight - TextureMoverTop->getHeight() - TextureMoverBottom->getHeight());
			TextureMoverBottom->RenderTexture(x + DirectionalButtonsX + 1, y + ContentY + UpButtonH + MoverY + MoverHeight - TextureMoverBottom->getHeight(), BarColumnW - 2, TextureMoverBottom->getHeight());
		}

		//  Render the items contained within
		for (int i = MovementIndex; i < int(m_ItemList.size()) && i < MovementIndex + ItemDisplayCount; ++i)
		{
			m_ItemList[i]->Render(x, y + ((EntryHeight + SpaceBetweenEntries) * (i - MovementIndex)));
		}

		for (int i = 0; i < int(m_ItemList.size()) && i < ItemDisplayCount ; ++i)
		{
			//m_ItemList[i]->Render(x, y + i * (EntryHeight + SpaceBetweenEntries));
		}

		//  Render the selector if an item is selected
		if (SelectedIndex != -1 && (SelectedIndex >= MovementIndex) && (SelectedIndex < MovementIndex + int(ItemDisplayCount)))
		{
			auto width = (renderScrollButtons ? (DirectionalButtonsX - TextureLeftSide->getWidth()) : m_Width - TextureLeftSide->getWidth() - TextureRightSide->getWidth()) - 2;
			TextureSelector->RenderTexture(x + TextureLeftSide->getWidth(), y + TextureTopSide->getHeight() + (EntryHeight + SpaceBetweenEntries) * (SelectedIndex - MovementIndex), width, EntryHeight);
		}
	}

	//  Pass the render call to all children
	for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter) (*iter)->Render(x, y);
}

void GUIListBox::UpdateMover()
{
	auto mover_size_percent = float(ItemDisplayCount) / float(m_ItemList.size());
	MoverHeight = (unsigned int)((float(m_Height - ContentY - DownButtonH) - (ContentY + UpButtonH)) * mover_size_percent);

	auto mover_position_percent_delta = float(1) / float(m_ItemList.size());
	MoverYDelta = (unsigned int)((float(m_Height - ContentY - DownButtonH) - (ContentY + UpButtonH)) * mover_position_percent_delta);
	MoverY = (unsigned int)(float(MoverYDelta) * float(MovementIndex));
}