#pragma once

#include "TextureManager.h"

#include <deque>
#include <assert.h>
#include <map>

class GUIObjectNode
{
protected:
	std::deque<GUIObjectNode*> m_Children;

public:
	static GUIObjectNode* CreateObjectNode(const char* imageFile);

	GUIObjectNode();
	virtual ~GUIObjectNode() {}

	virtual void Input(int xOffset = 0, int yOffset = 0);
	virtual void Render(int xOffset = 0, int yOffset = 0);
	virtual void SetToDestroy(std::map<GUIObjectNode*, bool>& destroyList);
	virtual void Destroy();

	void SetX(int x) { m_X = x; }
	void SetY(int y) { m_Y = y; }
	void SetWidth(int width) { m_Width = width; }
	void SetHeight(int height) { m_Height = height; }
	void SetTextureID(int textureID) { m_TextureID = textureID; }
	void SetVisible(bool visible) { m_Visible = visible; }
	void SetParent(GUIObjectNode* parent) { m_Parent = parent; }

	int GetX() const { return m_X; }
	int GetY() const { return m_Y; }
	int GetWidth() const { return m_Width; }
	int GetHeight() const { return m_Height; }

	void AddChild(GUIObjectNode* child);
	void RemoveChild(GUIObjectNode* child);
	
	int m_ZOrder;
	int m_X;
	int m_Y;
	int m_Width;
	int m_Height;
	GLuint m_TextureID;
	bool m_Visible;
	GUIObjectNode* m_Parent;
	bool m_SetToDestroy;
};

inline GUIObjectNode* GUIObjectNode::CreateObjectNode(const char* imageFile)
{
	auto newNode = new GUIObjectNode;
	newNode->SetTextureID(textureManager.LoadTextureGetID(imageFile));
	return newNode;
}

inline GUIObjectNode::GUIObjectNode() :
	m_ZOrder(0),
	m_X(0),
	m_Y(0),
	m_Width(0),
	m_Height(0),
	m_TextureID(0),
	m_Visible(true),
	m_Parent(nullptr),
	m_SetToDestroy(false)
{

}

inline void GUIObjectNode::Input(int xOffset, int yOffset)
{
	if (m_SetToDestroy || !m_Visible) return;

	//  Pass the input call to all children
	for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter) (*iter)->Input(xOffset + m_X, yOffset + m_Y);
}

inline void GUIObjectNode::Render(int xOffset, int yOffset)
{
	auto x = m_X + xOffset;
	auto y = m_Y + yOffset;

	//  Render the object if we're able
	if (!m_SetToDestroy && m_Visible && m_TextureID != 0 && m_Width > 0 && m_Height > 0)
	{
		glBindTexture(GL_TEXTURE_2D, m_TextureID);

		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex2i(x, y);
		glTexCoord2f(1.0f, 0.0f); glVertex2i(x + m_Width, y);
		glTexCoord2f(1.0f, 1.0f); glVertex2i(x + m_Width, y + m_Height);
		glTexCoord2f(0.0f, 1.0f); glVertex2i(x, y + m_Height);
		glEnd();
	}

	//  Pass the render call to all children
	for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter) (*iter)->Render(x, y);
}

inline void GUIObjectNode::SetToDestroy(std::map<GUIObjectNode*, bool>& destroyList)
{
	m_SetToDestroy = true;
	destroyList[this] = true;

	//  Pass the 'set to destroy' call to all children
	for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter) (*iter)->SetToDestroy(destroyList);
}

inline void GUIObjectNode::Destroy()
{
	if (m_Parent != nullptr) m_Parent->RemoveChild(this);

	//  Pass the destroy call to all children
	while (!m_Children.empty())
	{
		auto child = m_Children.back();
		m_Children.pop_back();
		child->SetParent(nullptr);
		child->Destroy();
	}
}

inline void GUIObjectNode::AddChild(GUIObjectNode* child)
{
	assert(!m_SetToDestroy);

	m_Children.push_back(child);
	child->m_Parent = this;
}

inline void GUIObjectNode::RemoveChild(GUIObjectNode* child)
{
	for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter)
	{
		if ((*iter) != child) continue;
		m_Children.erase(iter);
		return;
	}
}
