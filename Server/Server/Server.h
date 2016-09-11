#pragma once

#include "WinsockWrapper.h"

#include <vector>

#define MSL_IP			"127.0.0.1"//"fullsailchat.hopto.org"
#define MSL_PORT		2345

#define SERVER_PORT		2347

class Server
{
private:
	struct SocketData
	{
		SocketData(int socketID, std::string ipAddress, std::string name) :
			m_SocketID(socketID),
			m_IPAddress(ipAddress),
			m_Name(name),
			m_PingCount(0)
		{}

		int m_SocketID;
		std::string m_IPAddress;
		std::string m_Name;
		char m_PingCount;
	};

	bool Initialized;
	int ServerSocket;
	int MSLSocket;

	std::string m_Name;
	unsigned int m_ClientMax;

	std::vector<SocketData> m_ClientSocketDataList;
	bool m_ChangedThisFrame;

public:
	Server(void) : Initialized(false) {}
	~Server(void) {}

	bool Initialize(std::string serverName = "TEST SERVER", unsigned int maxClients = 4);
	bool MainProcess(void);
	void Shutdown(void);

	bool Messages_MSL(void);

	void AddClient(int socketID, std::string IP);
	void RemoveClient(int index);
	void AcceptNewClients(void);
	void Messages_Clients(void);

	void SendChatString(char* String);

	inline const char* GetClientIP(int i) { return m_ClientSocketDataList[i].m_IPAddress.c_str(); }
	inline int GetMSLSocket(void) { return MSLSocket; }
	inline std::string GetName() const { return m_Name; }
	inline unsigned int GetClientCount() const { return (unsigned int)(m_ClientSocketDataList.size()); }
	inline unsigned int GetClientMax() const { return m_ClientMax; }
};

bool Server::Initialize(std::string serverName, unsigned int maxClients)
{
	if (Initialized) return false;

	// Initialize the Listening Port for Clients
	ServerSocket = winsockWrapper.TCPListen(SERVER_PORT, 10, 1);
	if (ServerSocket == -1) return false;
	winsockWrapper.SetNagle(ServerSocket, true);

	// Connect to the MSL
	MSLSocket = winsockWrapper.TCPConnect(MSL_IP, MSL_PORT, 1);
	if (MSLSocket == -1) return false;

	m_Name = serverName;
	m_ClientMax = maxClients;
	m_ChangedThisFrame = true;
	Initialized = true;

	return true;
}

bool Server::MainProcess(void)
{
	// Accept Incoming Connections
	AcceptNewClients();

	// Receive messages
	if (Messages_MSL() == 0) return false;
	Messages_Clients();

	return true;
}

void Server::Shutdown(void)
{
	if (Initialized == false) return;

	Initialized = false;
	winsockWrapper.CloseSocket(ServerSocket);
	ServerSocket = -1;
	winsockWrapper.CloseSocket(MSLSocket);
	MSLSocket = -1;

	m_Name = "";
	m_ClientMax = 0;

	m_ClientSocketDataList.clear();
	m_ChangedThisFrame = false;
}

bool Server::Messages_MSL(void)
{
	if (!Initialized) return false;

	//  If we've changed anything, send the server data to MSL to be updated
	if (m_ChangedThisFrame)
	{
		winsockWrapper.ClearBuffer(0);
		winsockWrapper.WriteChar(2, 0);
		winsockWrapper.WriteString(m_Name.c_str(), 0);
		winsockWrapper.WriteUnsignedInt((unsigned int)(m_ClientSocketDataList.size()), 0);
		winsockWrapper.WriteUnsignedInt(m_ClientMax, 0);
		winsockWrapper.SendMessagePacket(MSLSocket, MSL_IP, MSL_PORT, 0);
	}

	int MessageBuffer = winsockWrapper.ReceiveMessagePacket(MSLSocket, 0, 0);
	if (MessageBuffer == 0) return false;
	if (MessageBuffer < 0) return true;

	char MessageID = winsockWrapper.ReadChar(0);

	switch (MessageID)
	{
	case 1:
		//  Ping message... send back a ping return so MSL will know we're alive
		winsockWrapper.ClearBuffer(0);
		winsockWrapper.WriteChar(1, 0);
		winsockWrapper.SendMessagePacket(MSLSocket, MSL_IP, MSL_PORT, 0);
		break;
	}

	return true;
}

////////////////////////////////////////
//	Client Connection Functions
////////////////////////////////////////

void Server::AddClient(int socketID, std::string ipAddress)
{
	m_ClientSocketDataList.push_back(SocketData(socketID, ipAddress, ""));
	m_ChangedThisFrame = true;
}

void Server::RemoveClient(int index)
{
	if (index >= int(m_ClientSocketDataList.size())) return;

	//  Look through the server list and if we find the one we're looking for, remove it
	for (auto iter = m_ClientSocketDataList.begin(); iter != m_ClientSocketDataList.end(); ++iter, --index)
	{
		if (index == 0)
		{
			m_ClientSocketDataList.erase(iter);
			m_ChangedThisFrame = true;
			return;
		}
	}
}

void Server::AcceptNewClients(void)
{
	if (!Initialized) return;

	int socketID = winsockWrapper.TCPAccept(ServerSocket, 1);
	while (socketID >= 0)
	{
		AddClient(socketID, winsockWrapper.GetExteriorIP(socketID).c_str());

		// Check for another client connection
		socketID = winsockWrapper.TCPAccept(ServerSocket, 1);
	}
}

void Server::Messages_Clients(void)
{
	for (auto i = 0; i < int(m_ClientSocketDataList.size()); i += 1)
	{
		int MessageBuffer = winsockWrapper.ReceiveMessagePacket(m_ClientSocketDataList[i].m_SocketID, 0, 0);
		if (MessageBuffer == 0)
		{
			RemoveClient(i);
			i -= 1;
			continue;
		}
		if (MessageBuffer < 0) continue;

		char MessageID = winsockWrapper.ReadChar(0);
		switch (MessageID)
		{
		case 1:
			m_ClientSocketDataList[i].m_PingCount = 0;
			break;

		case 2:
			SendChatString(winsockWrapper.ReadString(0));
			break;

		case 3:
			char NewString[100];
			char* Name = winsockWrapper.ReadString(0);
			char* Extra = " has entered the server.";
			int S = (int)strlen(Name);
			for (int i = 0; Name[i] != 0; i += 1)		NewString[i] = Name[i];
			for (int i = S; Extra[i - S] != 0; i += 1)	NewString[i] = Extra[i - S];
			S += (int)strlen(Extra);
			NewString[S] = 0;
			SendChatString(NewString);
		}
	}
}

////////////////////////////////////////
//	Program Functionality
////////////////////////////////////////

void Server::SendChatString(char* String)
{
	for (int i = 0; i < int(m_ClientSocketDataList.size()); i += 1)
	{
		winsockWrapper.ClearBuffer(0);
		winsockWrapper.WriteChar(2, 0);
		winsockWrapper.WriteString(String, 0);
		winsockWrapper.SendMessagePacket(m_ClientSocketDataList[i].m_SocketID, m_ClientSocketDataList[i].m_IPAddress.c_str(), SERVER_PORT, 0);
	}
}
