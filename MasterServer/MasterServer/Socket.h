#pragma once

#include "SocketBuffer.h"

#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

class Socket
{
	bool udp;
	int format;
	char formatstr[30];
	static SOCKADDR_IN SenderAddr;
	int receivetext(char*buf, int max);
public:

	SOCKET sockid;
	Socket(SOCKET sock);
	Socket();
	~Socket();
	bool tcpconnect(const char* address, int port, int mode);
	bool tcplisten(int port, int max, int mode);
	Socket* tcpaccept(int mode);
	std::string tcpip();
	void setnagle(bool enabled);
	bool tcpconnected();
	int setsync(int mode);
	bool udpconnect(int port, int mode);
	int sendmessage(const char* ip, int port, SocketBuffer* source);
	int receivemessage(int len, SocketBuffer*destination, int length_specific = 0);
	int peekmessage(int size, SocketBuffer*destination) const;
	static int lasterror();
	static std::string GetHostIP(char* address);
	static int SockExit(void);
	static int SockStart(void);
	static char* lastinIP(void);
	static unsigned short lastinPort(void);
	static char* myhost();
	int SetFormat(int mode, char* sep);
};

int SenderAddrSize = sizeof(SOCKADDR_IN);
SOCKADDR_IN Socket::SenderAddr;

inline bool Socket::tcpconnect(const char *address, int port, int mode)
{
	SOCKADDR_IN addr;
	if ((sockid = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == SOCKET_ERROR) return false;

	struct addrinfo hints, *servinfo;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(address, "http", &hints, &servinfo)) != 0)
	{
		closesocket(sockid);
		return false;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr = *((LPIN_ADDR)*servinfo->ai_addr->sa_data);
	addr.sin_port = htons((u_short)port);
	if (mode == 2) setsync(1);
	if (connect(sockid, (LPSOCKADDR)&addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			closesocket(sockid);
			return false;
		}
	
	if (mode == 1) setsync(1);
	return true;
}

inline bool Socket::tcplisten(int port, int max, int mode)
{
	if ((sockid = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) return false;
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	if (mode) setsync(1);
	if (bind(sockid, (LPSOCKADDR)&addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		closesocket(sockid);
		return false;
	}
	if (listen(sockid, max) == SOCKET_ERROR)
	{
		closesocket(sockid);
		return false;
	}
	return true;
}

inline Socket::Socket(SOCKET sock)
{
	sockid = sock;
	udp = false;
	format = 0;
}

inline Socket::Socket()
{
	udp = false;
	format = 0;
}

inline Socket::~Socket()
{
	if (sockid<0) return;
	shutdown(sockid, 1);
	closesocket(sockid);
}

inline Socket* Socket::tcpaccept(int mode)
{
	if (sockid<0) return 0;
	SOCKET sock2;
	if ((sock2 = accept(sockid, (SOCKADDR *)&SenderAddr, &SenderAddrSize)) != INVALID_SOCKET)
	{
		Socket*sockit = new Socket(sock2);
		if (mode >= 1)sockit->setsync(1);
		return sockit;
	}
	return 0;
}

inline std::string Socket::tcpip()
{
	if (sockid < 0) return nullptr;
	if (getpeername(sockid, (SOCKADDR *)&SenderAddr, &SenderAddrSize) == SOCKET_ERROR) return nullptr;

	char ipAddress[32];
	inet_ntop(AF_INET, &SenderAddr.sin_addr, ipAddress, INET_ADDRSTRLEN);
	return std::string(ipAddress);
}

inline void Socket::setnagle(bool enabled)
{
	if (sockid<0) return;
	setsockopt(sockid, IPPROTO_TCP, TCP_NODELAY, (char*)&enabled, sizeof(bool));
}

inline bool Socket::tcpconnected()
{
	if (sockid<0) return false;
	char b;
	if (recv(sockid, &b, 1, MSG_PEEK) == SOCKET_ERROR)
		if (WSAGetLastError() != WSAEWOULDBLOCK) return false;
	return true;
}

inline int Socket::setsync(int mode)
{
	if (sockid < 0) return -1;
	u_long i = mode;
	return ioctlsocket(sockid, FIONBIO, &i);
}

inline bool Socket::udpconnect(int port, int mode)
{
	SOCKADDR_IN addr;
	if ((sockid = socket(AF_INET, SOCK_DGRAM, 0)) == SOCKET_ERROR)
		return false;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	if (mode)setsync(1);
	if (bind(sockid, (SOCKADDR *)&addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		closesocket(sockid);
		return false;
	}
	udp = true;
	return true;
}

inline int Socket::sendmessage(const char *ip, int port, SocketBuffer *source)
{
	if (sockid <= 0) return -1;
	auto size = 0;
	SOCKADDR_IN addr;
	if (udp)
	{
		struct sockaddr_in sa;
		auto retVal = inet_pton(AF_INET, ip, &(sa.sin_addr));

		size = std::min(source->count, 8195);
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = sa.sin_addr.S_un.S_addr;
		size = sendto(sockid, source->data, size, 0, (SOCKADDR *)&addr, sizeof(SOCKADDR_IN));
	}
	else
	{
		SocketBuffer sendbuff;
		sendbuff.clear();
		if (format == 0)
		{
			sendbuff.writeushort(source->count);
			sendbuff.addBuffer(source);
			size = send(sockid, sendbuff.data, sendbuff.count, 0);
		}
		else if (format == 1)
		{
			sendbuff.addBuffer(source);
			sendbuff.writechars(formatstr);
			size = send(sockid, sendbuff.data, sendbuff.count, 0);
		}
		else if (format == 2)
			size = send(sockid, source->data, source->count, 0);
	}
	return ((size = SOCKET_ERROR) ? -WSAGetLastError() : size);
}

inline int Socket::receivetext(char*buf, int max)
{
	int len = (int)strlen(formatstr);
	if ((max = recv(sockid, buf, max, MSG_PEEK)) != SOCKET_ERROR)
	{
		int i, ii;
		for (i = 0; i < max; i++)
		{
			for (ii = 0; ii < len; ii++)
				if (buf[i + ii] != formatstr[ii]) break;
			if (ii == len)
				return recv(sockid, buf, i + len, 0);
		}
	}
	return -1;
}
inline int Socket::receivemessage(int len, SocketBuffer*destination, int length_specific)
{
	if (sockid<0) return -1;
	int size = -1;
	char* buff = NULL;
	if (udp)
	{
		size = 8195;
		buff = new char[size];
		size = recvfrom(sockid, buff, size, 0, (SOCKADDR *)&SenderAddr, &SenderAddrSize);
	}
	else
	{
		if (format == 0 && !len)
		{
			unsigned short length;
			if (length_specific == 0)
			{
				if (size = recv(sockid, (char*)&length, 2, 0) == SOCKET_ERROR) { return -1; }
				if (size = 0) { return 0; }
			}
			int buffer_size = length_specific != 0 ? length_specific : length;
			buff = new char[buffer_size];
			size = recv(sockid, buff, buffer_size, 0);
		}
		else if (format == 1 && !len)
		{
			size = 65536;
			buff = new char[size];
			size = receivetext(buff, size);
		}
		else if (format == 2 || len > 0)
		{
			buff = new char[len];
			size = recv(sockid, buff, len, 0);
		}
	}
	if (size > 0)
	{
		destination->clear();
		destination->addBuffer(buff, size);
	}
	if (buff != NULL) delete buff;
	return size;
}

inline int Socket::peekmessage(int size, SocketBuffer* destination) const
{
	if (sockid<0) return -1;
	if (size == 0) size = 65536;
	char *buff = new char[size];
	size = recvfrom(sockid, buff, size, MSG_PEEK, (SOCKADDR *)&SenderAddr, &SenderAddrSize);
	if (size < 0)
	{
		delete buff;
		return -1;
	}
	destination->clear();
	destination->addBuffer(buff, size);
	delete buff;
	return size;
}

inline int Socket::lasterror()
{
	return WSAGetLastError();
}

inline std::string Socket::GetHostIP(char* hostname)
{
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_in *h;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(hostname, "http", &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return "";
	}

	char ipAddress[32];

	// loop through all the results and connect to the first we can
	for (p = servinfo; p != nullptr; p = p->ai_next)
	{
		h = (struct sockaddr_in *) p->ai_addr;
		inet_ntop(AF_INET, &h->sin_addr, ipAddress, INET_ADDRSTRLEN);
	}

	freeaddrinfo(servinfo); // all done with this structure
	return std::string(ipAddress);
}

inline char* Socket::lastinIP(void)
{
	return "";
	////return inet_ntoa(SenderAddr.sin_addr);
}

inline unsigned short Socket::lastinPort(void)
{
	return ntohs(SenderAddr.sin_port);
}

inline int Socket::SetFormat(int mode, char* sep)
{
	int previous = format;
	format = mode;
	if (mode == 1 && strlen(sep)>0) strcpy_s(formatstr, 30, sep);
	return previous;
}

inline int Socket::SockExit(void)
{
	WSACleanup();
	return 1;
}

inline int Socket::SockStart(void)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(1, 1), &wsaData);
	return 1;
}

inline char* Socket::myhost()
{
	static char buf[16];
	gethostname(buf, 16);
	return buf;
}