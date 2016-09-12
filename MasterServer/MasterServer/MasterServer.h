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
			m_Clients(0),
			m_MaxClients(0),
			m_PingCount(0)
		{}

		int m_SocketID;
		std::string m_IPAddress;
		std::string m_Name;
		unsigned int m_Clients;
		unsigned int m_MaxClients;
		char m_PingCount;
	};

	int ServerSocket[2];

	std::vector<SocketData> m_ServerSocketDataList;
	std::vector<SocketData> m_ClientSocketDataList;
	bool m_ChangedThisFrame;

public:
	ServerList(void) : m_ChangedThisFrame(false) {}
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
	inline int GetServerConnectSocket()							const { return ServerSocket[SERVERS]; }
	inline int GetClientConnectSocket()							const { return ServerSocket[CLIENTS]; }
	inline bool GetChangedThisFrame()							const { return m_ChangedThisFrame; }
	inline unsigned int GetServerCount()						const { return m_ServerSocketDataList.size(); }
	inline unsigned int GetClientCount()						const { return m_ClientSocketDataList.size(); }
	inline int GetServerSocket(int index)					const { return m_ServerSocketDataList[index].m_SocketID; }
	inline const std::string& GetServerIP(int index)			const { return m_ServerSocketDataList[index].m_IPAddress; }
	inline const std::string& GetServerName(int index)			const { return m_ServerSocketDataList[index].m_Name; }
	inline unsigned int GetServerClientCount(int index)	const { return m_ServerSocketDataList[index].m_Clients; }
	inline unsigned int GetServerClientMax(int index)		const { return m_ServerSocketDataList[index].m_MaxClients; }
	inline const std::string& GetClientIP(int index)			const { return m_ClientSocketDataList[index].m_IPAddress; }
};


inline bool ServerList::Initialize(void)
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

inline void ServerList::MainProcess(void)
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

inline void ServerList::Shutdown(void)
{
	winsockWrapper.CloseSocket(ServerSocket[SERVERS]);
	winsockWrapper.CloseSocket(ServerSocket[CLIENTS]);
}


////////////////////////////////////////
//	Server Connection Functions
////////////////////////////////////////

inline void ServerList::AddServer(int socketID, std::string ipAddress, std::string name)
{
	m_ServerSocketDataList.push_back(SocketData(socketID, ipAddress, name));
	m_ChangedThisFrame = true;
}

inline void ServerList::RemoveServer(int index)
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



inline void ServerList::PingServers()
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

inline void ServerList::AcceptNewServers(void)
{
	auto socketID = winsockWrapper.TCPAccept(ServerSocket[SERVERS], 1);
	while (socketID >= 0)
	{
		AddServer(socketID, winsockWrapper.GetExteriorIP(socketID), "::NAME UNKNOWN::");

		// Check for another server connection
		socketID = winsockWrapper.TCPAccept(ServerSocket[SERVERS], 1);
	}
}

inline void ServerList::Messages_Servers(void)
{
	for (auto i = 0; i < int(m_ServerSocketDataList.size()); ++i)
	{
		auto MessageBuffer = winsockWrapper.ReceiveMessagePacket(m_ServerSocketDataList[i].m_SocketID, 0, 0);
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
			m_ServerSocketDataList[i].m_Clients = winsockWrapper.ReadUnsignedInt(0);
			m_ServerSocketDataList[i].m_MaxClients = winsockWrapper.ReadUnsignedInt(0);
			m_ServerSocketDataList[i].m_PingCount = 0;
			m_ChangedThisFrame = true;
			break;

		default:break;
		}
	}
}

////////////////////////////////////////
//	Client Connection Functions
////////////////////////////////////////

inline void ServerList::AddClient(int socketID, std::string ipAddress)
{
	m_ClientSocketDataList.push_back(SocketData(socketID, ipAddress, ""));
	m_ChangedThisFrame = true;
}

inline void ServerList::RemoveClient(int index)
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

inline void ServerList::PingClients()
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

inline void ServerList::AcceptNewClients(void)
{
	auto socketID = winsockWrapper.TCPAccept(ServerSocket[CLIENTS], 1);
	while (socketID >= 0)
	{
		AddClient(socketID, winsockWrapper.GetExteriorIP(socketID).c_str());

		// Check for another client connection
		socketID = winsockWrapper.TCPAccept(ServerSocket[CLIENTS], 1);
	}
}

inline void ServerList::Messages_Clients(void)
{
	for (auto i = 0; i < int(m_ClientSocketDataList.size()); ++i)
	{
		auto MessageBuffer = winsockWrapper.ReceiveMessagePacket(m_ClientSocketDataList[i].m_SocketID, 0, 0);
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
		case 0:
			SendServerList(m_ClientSocketDataList[i].m_SocketID);
			break;
		case 1:
			m_ClientSocketDataList[i].m_PingCount = 0;
			break;

		default:break;
		}
	}
}

inline void ServerList::SendServerList(int socketID)
{
	winsockWrapper.ClearBuffer(0);
	winsockWrapper.WriteChar(0, 0);
	for (auto i = 0; i < int(m_ServerSocketDataList.size()); ++i)
	{
		winsockWrapper.WriteUnsignedInt(winsockWrapper.ConvertIPtoUINT(m_ServerSocketDataList[i].m_IPAddress.c_str()), 0);
		winsockWrapper.WriteString(m_ServerSocketDataList[i].m_Name.c_str(), 0);
		winsockWrapper.WriteUnsignedInt(m_ServerSocketDataList[i].m_Clients, 0);
		winsockWrapper.WriteUnsignedInt(m_ServerSocketDataList[i].m_MaxClients, 0);
	}
	winsockWrapper.WriteUnsignedInt(winsockWrapper.ConvertIPtoUINT("0.0.0.0"), 0);
	winsockWrapper.SendMessagePacket(socketID, winsockWrapper.GetExteriorIP(socketID).c_str(), CLIENT_PORT, 0);
}