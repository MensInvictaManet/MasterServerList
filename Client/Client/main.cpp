#include "ArcadiaEngine.h"
#include "Client.h"

Client CLIENT;

GUILabel* serverListLabel;
GUIListBox* serverListBox;
GUIButton* serverListRefreshButton;
GUIButton* serverListPickButton;
GUIListBox* serverChatListBox;
GUILabel* inputNameLabel;
GUIEditBox* serverChatMessageEditBox;
GUIButton* serverSendMessageButton;
GUIButton* serverDisconnectButton;
int chatStringDisplayIndex = 0;

void DisconnectFromServer()
{
	CLIENT.Shutdown();
	CLIENT.Initialize();

	//  Hide the Server Chat UI
	inputNameLabel->SetVisible(false);
	serverChatListBox->SetVisible(false);
	serverChatMessageEditBox->SetVisible(false);
	serverSendMessageButton->SetVisible(false);
	serverDisconnectButton->SetVisible(false);

	//  Show the Server List UI
	serverListBox->SetSelectedIndex(-1);
	serverListLabel->SetVisible(true);
	serverListBox->SetVisible(true);
	serverListRefreshButton->SetVisible(true);
	serverListPickButton->SetVisible(true);
}

void AttemptToConnect()
{
	//  If we don't have a valid selected index in the server list, return out
	auto index = serverListBox->GetSelectedIndex();
	if (index == -1 || index > int(CLIENT.GetServerList().size())) return;

	//  If we fail to connect to the server, return out
	if (!CLIENT.ConnectToServer(CLIENT.GetServerList()[index].m_ServerIP.c_str())) return;
	CLIENT.SetServerIP(CLIENT.GetServerList()[index].m_ServerIP);

	//  Hide the Server List UI
	serverListLabel->SetVisible(false);
	serverListBox->SetVisible(false);
	serverListRefreshButton->SetVisible(false);
	serverListPickButton->SetVisible(false);
	CLIENT.ClearServerList();

	//  Show the Server Chat UI
	inputNameLabel->SetVisible(true);
	serverChatListBox->SetVisible(false);
	serverChatMessageEditBox->SetVisible(true);
	serverSendMessageButton->SetVisible(true);
	serverDisconnectButton->SetVisible(true);
}

void CreateProgramData()
{
	windowManager.SetWindowCaption(0, "MSL Client");

	//  Load the fonts for the UI
	fontManager.SetFontFolder("Fonts/");
	fontManager.LoadFont("Arial");
	fontManager.LoadFont("Arial-12-White");

	//  Create the Server List UI
	serverListLabel = GUILabel::CreateLabel(fontManager.GetFont("Arial"), "Select a server:", 16, 20, 100, 22);
	guiManager.GetBaseNode()->AddChild(serverListLabel);

	serverListBox = GUIListBox::CreateTemplatedListBox("Standard", 12, 40, 610, 684, 586, 4, 16, 16, 16, 16, 16, 22, 2);
	guiManager.GetBaseNode()->AddChild(serverListBox);

	serverListRefreshButton = GUIButton::CreateTemplatedButton("Standard", 12, 726, 300, 30);
	serverListRefreshButton->SetFont(fontManager.GetFont("Arial"));
	serverListRefreshButton->SetText("Refresh Server List");
	serverListRefreshButton->SetLeftClickCallback([=](GUIObjectNode*) { serverListBox->SetSelectedIndex(-1); CLIENT.RequestServerList(); });
	guiManager.GetBaseNode()->AddChild(serverListRefreshButton);

	serverListPickButton = GUIButton::CreateTemplatedButton("Standard", 322, 726, 300, 30);
	serverListPickButton->SetFont(fontManager.GetFont("Arial"));
	serverListPickButton->SetText("Connect To Server");
	serverListPickButton->SetLeftClickCallback([=](GUIObjectNode*) { AttemptToConnect(); });
	guiManager.GetBaseNode()->AddChild(serverListPickButton);

	//  Create the Server Chat UI but make it invisible
	inputNameLabel = GUILabel::CreateLabel(fontManager.GetFont("Arial"), "Input a name and hit Send Message to begin", 12, 700, 200, 22);
	inputNameLabel->SetVisible(false);
	guiManager.GetBaseNode()->AddChild(inputNameLabel);

	serverChatListBox = GUIListBox::CreateTemplatedListBox("Standard", 12, 40, 1000, 684, 980, 4, 16, 16, 16, 16, 16, 22, 2);
	serverChatListBox->SetSelectable(false);
	serverChatListBox->SetVisible(false);
	serverChatListBox->SetFlowToBottom(true);
	guiManager.GetBaseNode()->AddChild(serverChatListBox);

	serverChatMessageEditBox = GUIEditBox::CreateTemplatedEditBox("Standard", 12, 726, 572, 30);
	serverChatMessageEditBox->SetFont(fontManager.GetFont("Arial"));
	serverChatMessageEditBox->SetVisible(false);
	guiManager.GetBaseNode()->AddChild(serverChatMessageEditBox);

	serverSendMessageButton = GUIButton::CreateTemplatedButton("Standard", 588, 726, 200, 30);
	serverSendMessageButton->SetFont(fontManager.GetFont("Arial"));
	serverSendMessageButton->SetText("Send Message");
	serverSendMessageButton->SetLeftClickCallback([=](GUIObjectNode*)
	{
		if (CLIENT.GetLocalName().compare("") == 0)
		{
			CLIENT.SetLocalName(serverChatMessageEditBox->GetText());
			inputNameLabel->SetVisible(false);
			serverChatListBox->SetVisible(true);
		}
		else CLIENT.SendChatMessage(serverChatMessageEditBox->GetText());
		serverChatMessageEditBox->SetText("");
	});
	serverSendMessageButton->SetVisible(false);
	guiManager.GetBaseNode()->AddChild(serverSendMessageButton);

	serverDisconnectButton = GUIButton::CreateTemplatedButton("Standard", 792, 726, 220, 30);
	serverDisconnectButton->SetFont(fontManager.GetFont("Arial"));
	serverDisconnectButton->SetText("Disconnect From Server");
	serverDisconnectButton->SetLeftClickCallback([=](GUIObjectNode*) { DisconnectFromServer(); });
	serverDisconnectButton->SetVisible(false);
	guiManager.GetBaseNode()->AddChild(serverDisconnectButton);
}

void AddServerDisplay(std::string& serverName, std::string& serverIP, unsigned int clientCount, unsigned int clientMax)
{
	auto entryNode = new GUIObjectNode;

	auto serverNameLabel = GUILabel::CreateLabel(fontManager.GetFont("Arial"), serverName.c_str(), 10, 8, 100, 22);
	serverNameLabel->SetJustification(GUILabel::JUSTIFY_LEFT);
	entryNode->AddChild(serverNameLabel);

	auto serverIPLabel = GUILabel::CreateLabel(fontManager.GetFont("Arial"), serverIP.c_str(), 340, 8, 100, 22);
	serverIPLabel->SetJustification(GUILabel::JUSTIFY_CENTER);
	entryNode->AddChild(serverIPLabel);

	char playersString[16];
	sprintf_s(playersString, 16, "[%d / %d]", clientCount, clientMax);
	auto serverPlayersLabel = GUILabel::CreateLabel(fontManager.GetFont("Arial"), playersString, 536, 8, 60, 22);
	serverPlayersLabel->SetJustification(GUILabel::JUSTIFY_LEFT);
	entryNode->AddChild(serverPlayersLabel);

	serverListBox->AddItem(entryNode);
}

void UpdateServerListDisplay()
{
	if (CLIENT.GetClientState() == Client::CLIENT_STATE_DISCONNECTED && CLIENT.GetChangedThisFrame())
	{
		serverListBox->ClearItems();

		auto serverList = CLIENT.GetServerList();
		for (auto iter = serverList.begin(); iter != serverList.end(); ++iter)
		{
			AddServerDisplay((*iter).m_ServerName, (*iter).m_ServerIP, (*iter).m_Clients, (*iter).m_ClientsMax);
		}
	}
}

void UpdateChatDisplay()
{
	if (CLIENT.GetClientState() == Client::CLIENT_STATE_CONNECTED && CLIENT.GetChangedThisFrame())
	{
		auto arial = fontManager.GetFont("Arial");
		for (; chatStringDisplayIndex < CLIENT.GetChatStringCount(); ++chatStringDisplayIndex)
		{
			auto chatString = CLIENT.GetChatString(chatStringDisplayIndex);
			serverChatListBox->AddItem(GUILabel::CreateLabel(arial, std::string(chatString->m_Name + ": " + chatString->m_Message).c_str(), 10, 4, serverChatListBox->GetWidth(), serverChatListBox->GetHeight()));
		}
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
		CLIENT.MainProcess();
		UpdateServerListDisplay();
		UpdateChatDisplay();

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

	if (!CLIENT.Initialize())
	{
		ShutdownEngine();
		return 2;
	}

	//  Create test data for different systems to ensure they work as they should
	CreateProgramData();

	//  Begin the primary loop, and continue until it exits
	PrimaryLoop();

	//  Free resources and close SDL before exiting
	CLIENT.Shutdown();
	ShutdownEngine();
	return 0;
}