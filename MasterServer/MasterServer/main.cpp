#include "ArcadiaEngine.h"

#include "MasterServer.h"

ServerList MSL;
GUIListBox* serverListBox = nullptr;
GUIListBox* clientListBox = nullptr;

void AddServer(const char* serverName, int clientCount, int clientMax, const char* ipAddress, bool favorited)
{
	auto entryNode = new GUIObjectNode;

	auto serverNameLabel = GUILabel::CreateLabel(fontManager.GetFont("Arial"), serverName, 10, 8, 100, 22);
	serverNameLabel->SetJustification(GUILabel::JUSTIFY_LEFT);
	entryNode->AddChild(serverNameLabel);

	char playersString[16];
	sprintf_s(playersString, 16, "[%d / %d]", clientCount, clientMax);
	auto serverPlayersLabel = GUILabel::CreateLabel(fontManager.GetFont("Arial"), playersString, 340, 8, 60, 22);
	serverPlayersLabel->SetJustification(GUILabel::JUSTIFY_LEFT);
	entryNode->AddChild(serverPlayersLabel);

	auto serverIPLabel = GUILabel::CreateLabel(fontManager.GetFont("Arial"), ipAddress, 500, 8, 100, 22);
	serverIPLabel->SetJustification(GUILabel::JUSTIFY_CENTER);
	entryNode->AddChild(serverIPLabel);

	auto favoritedCheckbox = GUICheckbox::CreateTemplatedCheckbox("Standard", 630, 4, 22, 22);
	favoritedCheckbox->SetChecked(favorited);
	entryNode->AddChild(favoritedCheckbox);

	serverListBox->AddItem(entryNode);
}

void AddClient(const char* ipAddress)
{
	auto entryNode = new GUIObjectNode;

	auto clientIPLabel = GUILabel::CreateLabel(fontManager.GetFont("Arial"), ipAddress, 10, 8, 100, 22);
	clientIPLabel->SetJustification(GUILabel::JUSTIFY_LEFT);
	entryNode->AddChild(clientIPLabel);
	
	clientListBox->AddItem(entryNode);
}

void CreateProgramData()
{
	windowManager.SetWindowCaption(0, "MSL Master Server");

	//  Load the fonts for the UI
	fontManager.SetFontFolder("Fonts/");
	fontManager.LoadFont("Arial");
	fontManager.LoadFont("Arial-12-White");

	//  Load the listbox that will hold all of the current servers connected to the MSL
	guiManager.GetBaseNode()->AddChild(GUILabel::CreateLabel(fontManager.GetFont("Arial"), "Server List:", 6, 52, 100, 22));
	serverListBox = GUIListBox::CreateTemplatedListBox("Standard", 6, 70, 660, 526, 756, 6, 32, 32, 32, 32, 32, 22, 2);
	guiManager.GetBaseNode()->AddChild(serverListBox);

	//  Load the listbox that will hold all of the current servers connected to the MSL
	guiManager.GetBaseNode()->AddChild(GUILabel::CreateLabel(fontManager.GetFont("Arial"), "Client List:", 674, 52, 100, 22));
	clientListBox = GUIListBox::CreateTemplatedListBox("Standard", 674, 70, 120, 526, 756, 6, 32, 32, 32, 32, 32, 22, 2);
	guiManager.GetBaseNode()->AddChild(clientListBox);
}

void UpdateMSLDisplay()
{
	if (!MSL.GetChangedThisFrame()) return;

	//  Clear and repopulate the server list
	serverListBox->ClearItems();
	for (unsigned int i = 0, serverCount = MSL.GetServerCount(); i < serverCount; ++i)
	{
		AddServer(MSL.GetServerName(i).c_str(), MSL.GetServerClientCount(i), MSL.GetServerClientMax(i), MSL.GetServerIP(i).c_str(), false);
	}

	//  Clear and repopulate the client list
	clientListBox->ClearItems();
	for (unsigned int i = 0, clientCount = MSL.GetClientCount(); i < clientCount; ++i)
	{
		AddClient(MSL.GetClientIP(i).c_str());
	}
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
		MSL.MainProcess();
		UpdateMSLDisplay();

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

	if (!MSL.Initialize())
	{
		ShutdownEngine();
		return 2;
	}

	//  Create test data for different systems to ensure they work as they should
	CreateProgramData();

	//  Begin the primary loop, and continue until it exits
	PrimaryLoop();

	//  Free resources and close SDL before exiting
	MSL.Shutdown();
	ShutdownEngine();
	return 0;
}