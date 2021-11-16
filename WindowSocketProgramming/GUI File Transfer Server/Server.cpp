#include "Server.h"

void ErrorQuit(std::string msg)
{
	LPVOID lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPTSTR>(&lpMsgBuf), 0, nullptr);

	MessageBox(nullptr, static_cast<LPCTSTR>(lpMsgBuf), msg.c_str(), MB_ICONERROR);

	LocalFree(lpMsgBuf);
	exit(true);
}

bool CServer::Initialize()
{
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != NOERROR)
		return false;

	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (listenSocket == INVALID_SOCKET)
		ErrorQuit("socket()");

	ZeroMemory(&serverAddr, sizeof(serverAddr));

	serverAddr.sin_family		 = AF_INET;
	serverAddr.sin_addr.s_addr	 = htonl(INADDR_ANY);
	serverAddr.sin_port			 = htons(static_cast<u_short>(SERVER_INFO::SERVERPORT));

	if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
		ErrorQuit("bind()");

	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
		ErrorQuit("listen()");

	return true;
}
