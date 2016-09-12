#pragma once

#include "WinsockWrapper.h"

#define MSL_IP			"127.0.0.1"//"fullsailchat.hopto.org"
#define MSL_PORT		2346
#define SERVER_PORT		2347

class Client
{
public:
	enum ClientState { CLIENT_STATE_UNINITIALIZED, CLIENT_STATE_DISCONNECTED, CLIENT_STATE_CONNECTED };

private:
	struct ServerEntryData
	{
		std::string m_ServerIP;
		std::string m_ServerName;
		unsigned int m_Clients;
		unsigned int m_ClientsMax;
	};

	int m_ClientState;
	int m_MSLSocket;
	int m_ServerSocket;
	std::vector<ServerEntryData> m_ServerEntryList;

	std::string m_LocalName;
	bool m_ChangedThisFrame;

	void MSL_Messages();
	void Server_Messages();

public:
	Client() :
		m_ClientState(CLIENT_STATE_UNINITIALIZED),
		m_MSLSocket(-1),
		m_ServerSocket(-1),
		m_LocalName("TestName"),
		m_ChangedThisFrame(false)
	{}

	~Client() { Shutdown(); }

	void RequestServerList();
	bool ConnectToServer(const char* ipAddress);
	inline bool GetChangedThisFrame() const { return m_ChangedThisFrame; }
	inline int GetClientState() const { return m_ClientState; }
	inline const std::vector<ServerEntryData>& GetServerList() const { return m_ServerEntryList; }

	bool Initialize();
	bool MainProcess();
	void Shutdown();
};

void Client::MSL_Messages()
{
	int messageSize = winsockWrapper.ReceiveMessagePacket(m_MSLSocket, 0, 0);
	if (messageSize == 0) return; // TODO: Disconnect
	if (messageSize < 0) return;

	switch (winsockWrapper.ReadChar(0))
	{
	case 0: //  Server List data
		{
			m_ServerEntryList.clear();
			m_ChangedThisFrame = true;

			std::string ipAddress = winsockWrapper.ConvertUINTtoIP(winsockWrapper.ReadUnsignedInt(0));
			while (ipAddress.compare("0.0.0.0") != 0)
			{
				ServerEntryData newEntry;
				newEntry.m_ServerIP = ipAddress;
				newEntry.m_ServerName = winsockWrapper.ReadString(0);
				newEntry.m_Clients = winsockWrapper.ReadUnsignedInt(0);
				newEntry.m_ClientsMax = winsockWrapper.ReadUnsignedInt(0);
				m_ServerEntryList.push_back(newEntry);

				//  Get the next IP Address
				ipAddress = winsockWrapper.ConvertUINTtoIP(winsockWrapper.ReadUnsignedInt(0));
			}
		}
		break;

	case 1: //  Client Ping Request
		{
			winsockWrapper.ClearBuffer(0);
			winsockWrapper.WriteChar(1, 0);
			winsockWrapper.SendMessagePacket(m_MSLSocket, MSL_IP, MSL_PORT, 0);
		}
		break;
	}

}

void Client::Server_Messages()
{

}

void Client::RequestServerList()
{
	winsockWrapper.ClearBuffer(0);
	winsockWrapper.WriteChar(0, 0);
	winsockWrapper.SendMessagePacket(m_MSLSocket, MSL_IP, MSL_PORT, 0);
}

bool Client::ConnectToServer(const char* ipAddress)
{
	if (m_ClientState != CLIENT_STATE_DISCONNECTED) return false;

	//  Connect to the specified server
	m_ServerSocket = winsockWrapper.TCPConnect(ipAddress, SERVER_PORT, 1);
	if (m_ServerSocket == -1) return false;

	m_ServerEntryList.clear();
	m_ClientState = CLIENT_STATE_CONNECTED;
	return true;
}

bool Client::Initialize()
{
	if (m_ClientState != CLIENT_STATE_UNINITIALIZED) return false;

	//  Connect to the MSL
	m_MSLSocket = winsockWrapper.TCPConnect(MSL_IP, MSL_PORT, 1);
	if (m_MSLSocket == -1) return false;

	RequestServerList();
	m_ClientState = CLIENT_STATE_DISCONNECTED;
	return true;
}

bool Client::MainProcess()
{
	switch (m_ClientState)
	{
	case CLIENT_STATE_DISCONNECTED:
		MSL_Messages();
		return true;

	case CLIENT_STATE_CONNECTED:
		Server_Messages();
		return true;

	default:
	case CLIENT_STATE_UNINITIALIZED:
		return false;
		break;
	}
}

void Client::Shutdown()
{
	if (m_ClientState == CLIENT_STATE_UNINITIALIZED) return;

	if (m_MSLSocket != -1)
	{
		winsockWrapper.CloseSocket(m_MSLSocket);
		m_MSLSocket = -1;
	}

	if (m_ServerSocket != -1)
	{
		winsockWrapper.CloseSocket(m_ServerSocket);
		m_ServerSocket = -1;
	}

	m_LocalName = "";
	m_ChangedThisFrame = false;
}