#include "ArcadiaEngine.h"

#include "Server.h"

GUIEditBox* serverNameEditBox;
GUIButton* serverStartButton;
Server SERVER;

void CreateProgramData()
{
	//  Load the fonts for the UI
	fontManager.SetFontFolder("Fonts/");
	fontManager.LoadFont("Arial");
	fontManager.LoadFont("Arial-12-White");

	serverNameEditBox = GUIEditBox::CreateTemplatedEditBox("Standard", 10, 10, 300, 32);
	serverNameEditBox->SetFont(fontManager.GetFont("Arial"));
	guiManager.GetBaseNode()->AddChild(serverNameEditBox);

	serverStartButton = GUIButton::CreateTemplatedButton("Standard", 320, 10, 110, 32);
	serverStartButton->SetFont(fontManager.GetFont("Arial"));
	serverStartButton->SetText("Start Server");
	serverStartButton->SetLeftClickCallback([=](GUIObjectNode* object)
	{
		if (!serverNameEditBox->GetText().empty() && SERVER.Initialize())
		{
			// Send information to the MSL
			winsockWrapper.ClearBuffer(0);
			winsockWrapper.WriteChar(2, 0);
			winsockWrapper.WriteString(serverNameEditBox->GetText().c_str(), 0);
			winsockWrapper.SendMessagePacket(SERVER.GetMSLSocket(), MSL_IP, MSL_PORT, 0);
		}
	});
	guiManager.GetBaseNode()->AddChild(serverStartButton);
}

void RenderScreen()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//  Initiate the 3D Rendering Context
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, GLdouble(SCREEN_WIDTH) / GLdouble(SCREEN_HEIGHT), 1.0, 200.0);

	glDisable(GL_TEXTURE_2D);

	//  RENDER 3D START

	//  Initiate the 2D Rendering Context
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, GLdouble(SCREEN_WIDTH), GLdouble(SCREEN_HEIGHT), 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	//  RENDER 2D START

	//  Render the 2D GUI through the GUIManager
	guiManager.Render();
}

void PrimaryLoop()
{
	//  The event handler
	SDL_Event e;

	//  Main loop flag
	auto quit = false;

	//  While application is running
	while (!quit)
	{
		DetermineTimeSlice();

		//  Get the current state of mouse and keyboard input
		inputManager.GetInputForFrame();

		//  Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			switch (e.type)
			{
			case SDL_QUIT:
				quit = true;
				break;

			case SDL_TEXTINPUT:
				inputManager.AddKeyToString(e.text.text[0]);
				break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				switch (e.button.button)
				{
				case 1: inputManager.SetMouseButtonLeft((e.type == SDL_MOUSEBUTTONDOWN)); break;
				case 2: inputManager.SetMouseButtonMiddle((e.type == SDL_MOUSEBUTTONDOWN)); break;
				case 3: inputManager.SetMouseButtonRight((e.type == SDL_MOUSEBUTTONDOWN)); break;
				default: break;
				}

			default:
				windowManager.HandleEvent(e);
				break;
			}
		}

		//  Input
		//HandleInput();
		guiManager.Input();

		//  Update
		SERVER.MainProcess();

		//  Render
		RenderScreen();
		windowManager.Render();

		//  End Step
		guiManager.EndStep();
	}
}

int main(int argc, char* args[])
{
	//  Attempt to initialize OpenGL and SDL -- Close the program if that fails
	if (!InitializeEngine())
	{
		ShutdownEngine();
		return 1;
	}

	//  Create test data for different systems to ensure they work as they should
	CreateProgramData();

	//  Begin the primary loop, and continue until it exits
	PrimaryLoop();

	//  Free resources and close SDL before exiting
	SERVER.Shutdown();
	ShutdownEngine();
	return 0;
}