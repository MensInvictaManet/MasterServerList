#pragma once

#include "WinsockWrapper.h"
#include "TimeSlice.h"

#include <vector>

#define SERVERS			0
#define CLIENTS			1
#define MAX_PING		3

#define SERVER_PORT		2345
#define CLIENT_PORT		2346

class ServerList
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

	int ServerSocket[2];

	std::vector<SocketData> m_ServerSocketDataList;
	std::vector<SocketData> m_ClientSocketDataList;
	bool m_ChangedThisFrame;

public:
	ServerList(void) {}
	~ServerList(void) {}

	bool Initialize(void);
	void MainProcess(void);
	void Shutdown(void);

	void AddServer(int socketID, std::string ipAddress, std::string name);
	void RemoveServer(int index);
	void PingServers();
	void AcceptNewServers(void);
	void Messages_Servers(void);

	void AddClient(int socketID, std::string ipAddress);
	void RemoveClient(int index);
	void PingClients();
	void AcceptNewClients(void);
	void Messages_Clients(void);
	void SendServerList(int socketID);

	// Accessors
	inline bool GetChangedThisFrame()					const { return m_ChangedThisFrame; }
	inline unsigned int GetServerCount()				const { return m_ServerSocketDataList.size(); }
	inline unsigned int GetClientCount()				const { return m_ClientSocketDataList.size(); }
	inline const int GetServerSocket(int index)			const { return m_ServerSocketDataList[index].m_SocketID;	}
	inline const std::string& GetServerIP(int index)	const { return m_ServerSocketDataList[index].m_IPAddress;	}
	inline const std::string& GetServerName(int index)	const { return m_ServerSocketDataList[index].m_Name;		}
	inline const std::string& GetClientIP(int index)	const { return m_ClientSocketDataList[index].m_IPAddress;	}
};


bool ServerList::Initialize(void)
{
	// Initialize the Listening Port for Servers
	ServerSocket[SERVERS] = winsockWrapper.TCPListen(SERVER_PORT, 0, 1);
	if (ServerSocket[SERVERS] == -1) return false;
	winsockWrapper.SetNagle(ServerSocket[SERVERS], true);

	// Initialize the Listening Port for Clients
	ServerSocket[CLIENTS] = winsockWrapper.TCPListen(CLIENT_PORT, 0, 1);
	if (ServerSocket[CLIENTS] == -1) return false;
	winsockWrapper.SetNagle(ServerSocket[CLIENTS], true);

	return true;
}

void ServerList::MainProcess(void)
{
	m_ChangedThisFrame = false;

	// Ping all connections
	PingServers();
	PingClients();

	// Accept Incoming Connections
	AcceptNewServers();
	AcceptNewClients();

	// Receive messages
	Messages_Servers();
	Messages_Clients();
}

void ServerList::Shutdown(void)
{
	winsockWrapper.CloseSocket(ServerSocket[SERVERS]);
	winsockWrapper.CloseSocket(ServerSocket[CLIENTS]);
}


////////////////////////////////////////
//	Server Connection Functions
////////////////////////////////////////

void ServerList::AddServer(int socketID, std::string ipAddress, std::string name)
{
	m_ServerSocketDataList.push_back(SocketData(socketID, ipAddress, name));
	m_ChangedThisFrame = true;
}

void ServerList::RemoveServer(int index)
{
	if (index >= int(m_ServerSocketDataList.size())) return;

	//  Look through the server list and if we find the one we're looking for, remove it
	for (auto iter = m_ServerSocketDataList.begin(); iter != m_ServerSocketDataList.end(); ++iter, --index)
	{
		if (index == 0)
		{
			m_ServerSocketDataList.erase(iter);
			m_ChangedThisFrame = true;
			return;
		}
	}
}



void ServerList::PingServers()
{
	// Only run this every 5 seconds
	static float TIME_PingOffset = 0.0;
	TIME_PingOffset += tickSeconds;
	if (TIME_PingOffset < 5.0f) return;
	TIME_PingOffset -= 5.0f;

	// Send the ping character (1) to all servers
	for (auto i = 0; i < int(m_ServerSocketDataList.size()); ++i)
	{
		m_ServerSocketDataList[i].m_PingCount++;
		if (m_ServerSocketDataList[i].m_PingCount > MAX_PING)
		{
			RemoveServer(i);
			i -= 1;
			continue;
		}

		winsockWrapper.ClearBuffer(0);
		winsockWrapper.WriteChar(1, 0);
		winsockWrapper.SendMessagePacket(m_ServerSocketDataList[i].m_SocketID, m_ServerSocketDataList[i].m_IPAddress.c_str(), SERVER_PORT, 0);
	}
}

void ServerList::AcceptNewServers(void)
{
	int socketID = winsockWrapper.TCPAccept(ServerSocket[SERVERS], 1);
	while (socketID >= 0)
	{
		AddServer(socketID, winsockWrapper.GetIP(socketID), "::NAME UNKNOWN::");

		// Check for another server connection
		socketID = winsockWrapper.TCPAccept(ServerSocket[SERVERS], 1);
	}
}

void ServerList::Messages_Servers(void)
{
	for (auto i = 0; i < int(m_ServerSocketDataList.size()); ++i)
	{
		int MessageBuffer = winsockWrapper.ReceiveMessagePacket(m_ServerSocketDataList[i].m_SocketID, 0, 0);
		if (MessageBuffer == 0)
		{
			RemoveServer(i);
			i -= 1;
			continue;
		}
		if (MessageBuffer < 0) continue;

		char MessageID = winsockWrapper.ReadChar(0);

		switch (MessageID)
		{
		case 1:
			m_ServerSocketDataList[i].m_PingCount = 0;
			break;
		case 2:
			m_ServerSocketDataList[i].m_Name = winsockWrapper.ReadString(0);
			m_ServerSocketDataList[i].m_PingCount = 0;
			break;
		}
	}
}

////////////////////////////////////////
//	Client Connection Functions
////////////////////////////////////////

void ServerList::AddClient(int socketID, std::string ipAddress)
{
	m_ClientSocketDataList.push_back(SocketData(socketID, ipAddress, ""));
	m_ChangedThisFrame = true;
}

void ServerList::RemoveClient(int index)
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

void ServerList::PingClients()
{
	// Only run this every 5 seconds
	static float TIME_PingOffset = 0.0;
	TIME_PingOffset += tickSeconds;
	if (TIME_PingOffset < 5.0f) return;
	TIME_PingOffset -= 5.0f;

	// Send the ping character (1) to all servers
	for (auto i = 0; i < int(m_ClientSocketDataList.size()); ++i)
	{
		m_ClientSocketDataList[i].m_PingCount++;
		if (m_ClientSocketDataList[i].m_PingCount > MAX_PING)
		{
			RemoveClient(i);
			i -= 1;
			continue;
		}

		winsockWrapper.ClearBuffer(0);
		winsockWrapper.WriteChar(1, 0);
		winsockWrapper.SendMessagePacket(m_ClientSocketDataList[i].m_SocketID, m_ClientSocketDataList[i].m_IPAddress.c_str(), CLIENT_PORT, 0);
	}
}

void ServerList::AcceptNewClients(void)
{
	int socketID = winsockWrapper.TCPAccept(ServerSocket[CLIENTS], 1);
	while (socketID >= 0)
	{
		AddClient(socketID, winsockWrapper.GetIP(socketID).c_str());

		// Check for another client connection
		socketID = winsockWrapper.TCPAccept(ServerSocket[CLIENTS], 1);
	}
}

void ServerList::Messages_Clients(void)
{
	for (auto i = 0; i < int(m_ClientSocketDataList.size()); ++i)
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
			SendServerList(m_ClientSocketDataList[i].m_SocketID);
			break;
		}
	}
}

void ServerList::SendServerList(int socketID)
{
	winsockWrapper.ClearBuffer(0);
	for (auto i = 0; i < int(m_ServerSocketDataList.size()); ++i)
	{
		winsockWrapper.WriteUnsignedInt(winsockWrapper.ConvertIPtoUINT(m_ServerSocketDataList[i].m_IPAddress.c_str()), 0);
		const char* test = m_ServerSocketDataList[i].m_Name.c_str();
		winsockWrapper.WriteString(test, 0);
	}
	winsockWrapper.WriteUnsignedInt(winsockWrapper.ConvertIPtoUINT("0.0.0.0"), 0);
	winsockWrapper.SendMessagePacket(socketID, winsockWrapper.GetIP(socketID).c_str(), CLIENT_PORT, 0);
}