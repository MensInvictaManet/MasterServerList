#pragma once

#include "SDL2/SDL.h"
#pragma comment(lib, "SDL2/SDL2.lib")

enum MouseButtonState
{
	MOUSE_BUTTON_UNPRESSED,
	MOUSE_BUTTON_PRESSED,
	MOUSE_BUTTON_PRESSED_TAKEN,
	MOUSE_BUTTON_HELD
};

class InputManager
{
public:
	static InputManager& GetInstance() { static InputManager INSTANCE; return INSTANCE; }

	void GetInputForFrame();

	int GetMouseX() const { return m_MouseX; }
	int GetMouseY() const { return m_MouseY; }
	MouseButtonState GetMouseButtonLeft() const { return m_MouseButtonLeft; }
	MouseButtonState GetMouseButtonMiddle() const { return m_MouseButtonMiddle; }
	MouseButtonState GetMouseButtonRight() const { return m_MouseButtonRight; }
	void TakeMouseButtonLeft() { if (m_MouseButtonLeft == MOUSE_BUTTON_PRESSED) m_MouseButtonLeft = MOUSE_BUTTON_PRESSED_TAKEN; }
	void TakeMouseButtonMiddle() { if (m_MouseButtonMiddle == MOUSE_BUTTON_PRESSED) m_MouseButtonMiddle = MOUSE_BUTTON_PRESSED_TAKEN; }
	void TakeMouseButtonRight() { if (m_MouseButtonRight == MOUSE_BUTTON_PRESSED) m_MouseButtonRight = MOUSE_BUTTON_PRESSED_TAKEN; }
	bool GetKeyDown(const Uint8& sdlKey) const { return (m_KeyStates != nullptr) ? (m_KeyStates[sdlKey] != 0) : false; }
	bool GetBackspace() const { return m_KeyStates[SDL_SCANCODE_BACKSPACE] != 0; }
	const std::string& GetKeyboardString() const { return m_KeyboardString; }

	void SetMouseButtonLeft(bool setting) { if (setting != (m_MouseButtonLeft != MOUSE_BUTTON_UNPRESSED)) m_MouseButtonLeft = (setting ? MOUSE_BUTTON_PRESSED : MOUSE_BUTTON_UNPRESSED); }
	void SetMouseButtonMiddle(bool setting) { if (setting != (m_MouseButtonMiddle != MOUSE_BUTTON_UNPRESSED)) m_MouseButtonMiddle = (setting ? MOUSE_BUTTON_PRESSED : MOUSE_BUTTON_UNPRESSED); }
	void SetMouseButtonRight(bool setting) { if (setting != (m_MouseButtonRight != MOUSE_BUTTON_UNPRESSED)) m_MouseButtonRight = (setting ? MOUSE_BUTTON_PRESSED : MOUSE_BUTTON_UNPRESSED); }
	void AddKeyToString(int key);

private:
	InputManager();
	~InputManager();

	int m_MouseX;
	int m_MouseY;
	MouseButtonState m_MouseButtonLeft;
	MouseButtonState m_MouseButtonMiddle;
	MouseButtonState m_MouseButtonRight;
	const Uint8* m_KeyStates;
	std::string m_KeyboardString;
};

inline void InputManager::GetInputForFrame()
{
	m_KeyboardString = "";
	SDL_GetMouseState(&m_MouseX, &m_MouseY);
	if (m_MouseButtonLeft == MOUSE_BUTTON_PRESSED) m_MouseButtonLeft = MOUSE_BUTTON_HELD;
	if (m_MouseButtonMiddle == MOUSE_BUTTON_PRESSED) m_MouseButtonMiddle = MOUSE_BUTTON_HELD;
	if (m_MouseButtonRight == MOUSE_BUTTON_PRESSED) m_MouseButtonRight = MOUSE_BUTTON_HELD;
	m_KeyStates = SDL_GetKeyboardState(nullptr);
}

inline void InputManager::AddKeyToString(int key)
{
	if (key < 32) return;
	if ((key > 126) && ((key < 256) || (key > 265))) return;

	//  If the symbol is a letter and shift is held down, capitalize it
	if ((key >= 97) && (key <= 122) && (GetKeyDown(Uint8(303)) || GetKeyDown(Uint8(304))))
		key -= 32;

	if ((GetKeyDown(Uint8(303)) || GetKeyDown(Uint8(304))))
	{
		switch (key)
		{
		case 39:	key = 34;		break;
		case 44:	key = 60;		break;
		case 45:	key = 95;		break;
		case 46:	key = 62;		break;
		case 47:	key = 63;		break;
		case 48:	key = 41;		break;
		case 49:	key = 33;		break;
		case 50:	key = 64;		break;
		case 51:	key = 35;		break;
		case 52:	key = 36;		break;
		case 53:	key = 37;		break;
		case 54:	key = 94;		break;
		case 55:	key = 38;		break;
		case 56:	key = 42;		break;
		case 57:	key = 40;		break;
		case 59:	key = 58;		break;
		case 61:	key = 43;		break;
		case 91:	key = 123;		break;
		case 92:	key = 124;		break;
		case 93:	key = 125;		break;
		case 96:	key = 126;		break;
		default:break;
		}
	}

	switch (key)
	{
	case 256:
	case 257:
	case 258:
	case 259:
	case 260:
	case 261:
	case 262:
	case 263:
	case 264:
	case 265:
		key -= 208;
		break;
	default:break;
	}

	m_KeyboardString += (unsigned char)key;
}


inline InputManager::InputManager() :
	m_MouseX(0),
	m_MouseY(0),
	m_MouseButtonLeft(MOUSE_BUTTON_UNPRESSED),
	m_MouseButtonMiddle(MOUSE_BUTTON_UNPRESSED),
	m_MouseButtonRight(MOUSE_BUTTON_UNPRESSED),
	m_KeyStates(nullptr)
{

}

inline InputManager::~InputManager()
{

}

//  Instance to be utilized by anyone including this header
InputManager& inputManager = InputManager::GetInstance();