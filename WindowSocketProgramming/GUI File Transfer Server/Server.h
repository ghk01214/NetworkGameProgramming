#ifndef CSERVER_H_
#define CSERVER_H_

#pragma comment(lib, "ws2_32")
#pragma warning(disable : 4996)

#include <WinSock2.h>
#include <string>
#include <vector>
#include <windows.h>

enum class SERVER_INFO : int
{
	SERVERPORT = 9000, BUFSIZE = 512
};

class CServer
{
public:
	CServer() = default;
public:
	bool Initialize();
private:
	SOCKET listenSocket;
	sockaddr_in serverAddr;

	SOCKET clientSocket;
	sockaddr_in clientAddr;

	int cliAddrLen{ sizeof(clientAddr) };
	std::vector<HANDLE> clientThread;
};

#endif