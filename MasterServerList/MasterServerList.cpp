#include "ArcadiaEngine.h"

GUIListBox* serverListBox = nullptr;

void AddServer(const char* serverName, int playerMax, const char* ipAddress, bool favorited)
{
	GUIObjectNode* entryNode = new GUIObjectNode;

	GUILabel* serverNameLabel = GUILabel::CreateLabel(serverName, 10, 8, 100, 22);
	serverNameLabel->SetFont(fontManager.GetFont("Arial"));
	serverNameLabel->SetJustification(GUILabel::JUSTIFY_LEFT);
	entryNode->AddChild(serverNameLabel);

	char playersString[16];
	sprintf_s(playersString, 16, "[0 / %d]", playerMax);
	GUILabel* serverPlayersLabel = GUILabel::CreateLabel(playersString, 340, 8, 60, 22);
	serverPlayersLabel->SetFont(fontManager.GetFont("Arial"));
	serverPlayersLabel->SetJustification(GUILabel::JUSTIFY_LEFT);
	entryNode->AddChild(serverPlayersLabel);

	GUILabel* serverIPLabel = GUILabel::CreateLabel(ipAddress, 580, 8, 100, 22);
	serverIPLabel->SetFont(fontManager.GetFont("Arial"));
	serverIPLabel->SetJustification(GUILabel::JUSTIFY_CENTER);
	entryNode->AddChild(serverIPLabel);

	GUICheckbox* favoritedCheckbox = GUICheckbox::CreateTemplatedCheckbox("Standard", 760, 4, 22, 22);
	favoritedCheckbox->SetChecked(favorited);
	entryNode->AddChild(favoritedCheckbox);

	serverListBox->AddItem(entryNode);
}

void CreateMenuUI()
{
	//  Load the fonts for the UI
	fontManager.SetFontFolder("Fonts/");
	fontManager.LoadFont("Arial");
	fontManager.LoadFont("Arial-12-White");

	//  Load the listbox that will hold all of the current servers connected to the MSL
	serverListBox = GUIListBox::CreateTemplatedListBox("Standard", 6, 120, 788, 642, 756, 6, 32, 32, 32, 32, 32, 22, 2);
	guiManager.GetBaseNode()->AddChild(serverListBox);

	AddServer("Test Server Name", 10, "127.0.0.1", false);
	AddServer("/chicken Forever", 32, "192.168.0.1", true);
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
			//  User requests quit
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
			//  Handle keypress with current mouse position
			else if (e.type == SDL_TEXTINPUT)
			{
				inputManager.AddKeyToString(e.text.text[0]);
			}
			else
			{
				if (/*e.type == SDL_MOUSEMOTION || */e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP)
				{
					switch (e.button.button)
					{
					case 1: inputManager.SetMouseButtonLeft((e.type == SDL_MOUSEBUTTONDOWN)); break;
					case 2: inputManager.SetMouseButtonMiddle((e.type == SDL_MOUSEBUTTONDOWN)); break;
					case 3: inputManager.SetMouseButtonRight((e.type == SDL_MOUSEBUTTONDOWN)); break;
					default: break;
					}
				}
				windowManager.HandleEvent(e);
			}
		}

		//  Input
		//HandleInput();
		guiManager.Input();

		//  Update
		//  ?

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
	CreateMenuUI();

	//  Begin the primary loop, and continue until it exits
	PrimaryLoop();

	//  Free resources and close SDL before exiting
	ShutdownEngine();
	return 0;
}