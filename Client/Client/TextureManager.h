#pragma once

#include "SDL2/SDL.h"
#pragma comment(lib, "SDL2/SDL2.lib")

#include "SDL2/SDL_image.h"
#pragma comment(lib, "SDL2/SDL2_image.lib")

#include "SDL2/SDL_opengl.h"
#pragma comment(lib, "opengl32.lib")

#include <unordered_map>

#include "WindowManager.h"

class TextureManager
{
public:
	struct ManagedTexture
	{
	public:
		ManagedTexture(SDL_Texture* texture, GLuint textureID, int width, int height, int listIndex) :
			m_Texture(texture),
			m_TextureID(textureID),
			m_Width(width),
			m_Height(height),
			m_ListIndex(listIndex)
		{}

		~ManagedTexture()
		{
			FreeTexture();
		}

		//  TODO: SetColor (set color value and use it when rendering), Render
		/*
				//Set color modulation
				void setColor(Uint8 red, Uint8 green, Uint8 blue)
				{
					//Modulate texture
					SDL_SetTextureColorMod(mTexture, red, green, blue);
				}

				//Renders texture at given point
				void render(int x, int y, SDL_Rect* clip = NULL)
				{
					//Set rendering space and render to screen
					SDL_Rect renderQuad = { x, y, mWidth, mHeight };

					//Set clip rendering dimensions
					if (clip != NULL)
					{
						renderQuad.w = clip->w;
						renderQuad.h = clip->h;
					}

					//Render to screen
					SDL_RenderCopy(g_Renderer, mTexture, clip, &renderQuad);
				}
		 */

		void RenderTexture(int x, int y, int width = -1, int height = -1) const
		{
			if (m_TextureID == 0) return;
			if (width < 0) width = m_Width;
			if (height < 0) height = m_Height;

			glBindTexture(GL_TEXTURE_2D, m_TextureID);

			glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f); glVertex2i(x, y);
				glTexCoord2f(1.0f, 0.0f); glVertex2i(x + width, y);
				glTexCoord2f(1.0f, 1.0f); glVertex2i(x + width, y + height);
				glTexCoord2f(0.0f, 1.0f); glVertex2i(x, y + height);
			glEnd();
		}

		void RenderTexturePart(int x, int y, int sub_x, int sub_y, int sub_w, int sub_h) const
		{
			if (m_TextureID == 0) return;
			if (sub_w <= 0) sub_w = m_Width - sub_x;
			if (sub_h <= 0) sub_h = m_Height - sub_y;

			glBindTexture(GL_TEXTURE_2D, m_TextureID);

			glBegin(GL_QUADS);
				glTexCoord2f(float(sub_x) / (float(m_Width)), (float(sub_y) / float(m_Height)));
				glVertex3i(x, y, 0);
				glTexCoord2f((float(sub_x + sub_w) / float(m_Width)), (float(sub_y) / float(m_Height)));
				glVertex3i(x + sub_w, y, 0);
				glTexCoord2f((float(sub_x + sub_w) / float(m_Width)), (float(sub_y + sub_h) / float(m_Height)));
				glVertex3i(x + sub_w, y + sub_h, 0);
				glTexCoord2f((float(sub_x) / float(m_Width)), (float(sub_y + sub_h) / float(m_Height)));
				glVertex3i(x, y + sub_h, 0);
			glEnd();
		}

		void FreeTexture()
		{
			//  Free the texture if it exists
			if (m_Texture != nullptr)
			{
				SDL_DestroyTexture(m_Texture);
				m_Texture = nullptr;
				m_TextureID = 0;
				m_Width = 0;
				m_Height = 0;
			}
		}

		int getWidth() const { return m_Width; }
		int getHeight() const { return m_Height; }

		SDL_Texture* m_Texture;
		GLuint m_TextureID;
		int m_Width;
		int m_Height;
		int m_ListIndex;
	};

	static TextureManager& GetInstance() { static TextureManager INSTANCE; return INSTANCE; }

	GLuint LoadTextureGetID(const char* textureFile);
	ManagedTexture* LoadTexture(const char* textureFile);
	GLuint GetTextureID(const int index);
	ManagedTexture* GetManagedTexture(const int index);
	void Shutdown();

private:
	TextureManager();
	~TextureManager();

	int FirstFreeIndex();

	std::unordered_map<int, ManagedTexture*> m_TextureList;
	std::unordered_map<std::string, ManagedTexture*> m_TextureListByFile;
};

inline GLuint TextureManager::LoadTextureGetID(const char* textureFile)
{
	auto* texture = LoadTexture(textureFile);
	return (texture != nullptr) ? texture->m_TextureID : 0;
}


inline TextureManager::ManagedTexture* TextureManager::LoadTexture(const char* textureFile)
{
	auto iter = m_TextureListByFile.find(textureFile);
	if (iter != m_TextureListByFile.end()) return (*iter).second;

	//  TODO: Load the texture, create a ManagedTexture, return the Texture ID
	SDL_Texture* sdlTexture = nullptr;

	//  Load the surface from the given file
	auto sdlSurface = IMG_Load(textureFile);
	if (sdlSurface == nullptr)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", textureFile, IMG_GetError());
		return nullptr;
	}

	//  Color key the image (Aqua for transparency)
	SDL_SetColorKey(sdlSurface, SDL_TRUE, SDL_MapRGB(sdlSurface->format, 0, 0xFF, 0xFF));

	//  Create texture from surface pixels
	auto renderer = WindowManager::GetInstance().GetRenderer(0);  //  TODO: Find out if this is correct?
	sdlTexture = SDL_CreateTextureFromSurface(renderer, sdlSurface);
	if (sdlTexture == nullptr)
	{
		SDL_FreeSurface(sdlSurface);
		printf("Unable to create texture from %s! SDL Error: %s\n", textureFile, SDL_GetError());
		return nullptr;
	}

	//  Get the dimensions of the texture surface as well as the bpp value
	auto width = sdlSurface->w;
	auto height = sdlSurface->h;
	auto mode = (sdlSurface->format->BytesPerPixel == 4) ? GL_RGBA : GL_RGB;

	//  Create the OpenGL texture and push the surface data to it
	GLuint textureID = 0;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, mode, sdlSurface->w, sdlSurface->h, 0, mode, GL_UNSIGNED_BYTE, sdlSurface->pixels);

	//  Set the texture parameters for the loaded texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//  Get rid of the old loaded surface
	SDL_FreeSurface(sdlSurface);

	//  Create a ManagedTexture with our new data, and stick it in the TextureList
	auto index = FirstFreeIndex();
	auto managedTexture = new ManagedTexture(sdlTexture, textureID, width, height, index);
	if (managedTexture == nullptr)
	{
		glDeleteTextures(1, &textureID);
		printf("Unable to create ManagedTexture with data\n");
		return nullptr;
	}
	m_TextureList[index] = managedTexture;
	m_TextureListByFile[std::string(textureFile)] = managedTexture;

	return managedTexture;
}

inline GLuint TextureManager::GetTextureID(const int index)
{
	auto iter = m_TextureList.find(index);
	if (iter == m_TextureList.end()) return 0;

	return (*iter).second->m_TextureID;
}

inline TextureManager::ManagedTexture* TextureManager::GetManagedTexture(const int index)
{
	auto iter = m_TextureList.find(index);
	if (iter == m_TextureList.end()) return nullptr;

	return (*iter).second;
}

void TextureManager::Shutdown()
{
	
}

inline TextureManager::TextureManager()
{
	
}

inline TextureManager::~TextureManager()
{

}

inline int TextureManager::FirstFreeIndex()
{
	//  Find the first free index
	auto index = 0;
	for (; ; ++index)
	{
		auto iter = m_TextureList.find(index);
		if (iter == m_TextureList.end()) return index;
	}
}

//  Instance to be utilized by anyone including this header
TextureManager& textureManager = TextureManager::GetInstance();