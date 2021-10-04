#pragma comment(lib, "ws2_32")
#pragma warning(disable : 4996)

#include <iostream>
#include <string>
#include <fstream>
#include <WinSock2.h>
#include <WS2tcpip.h>

#define SERVERPORT	9000
#define BUFSIZE		512

// 출력된 바로 전 라인 제거
//std::cout << "\33[2K\x1B[A";

void ErrorQuit(std::string msg);
void DisplayError(std::string msg);
int recvns(SOCKET socket, std::string* buf, int nLength, int flags);
int recvnc(SOCKET socket, char* buf, int nLength, int flags);

int main()
{
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != NOERROR)
		return 1;

	SOCKET listenSocket{ socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) };

	if (listenSocket == INVALID_SOCKET)
		ErrorQuit("socket()");

	sockaddr_in serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(SERVERPORT);

	if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
		ErrorQuit("bind()");

	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
		ErrorQuit("listen()");

	SOCKET		 clientSocket;
	sockaddr_in	 clientAddr;
	int			 nClAddrLen;

	std::string	 sBuf;
	int			 sLen;

	while (true)
	{
		nClAddrLen = sizeof(clientAddr);
		clientSocket = accept(listenSocket, reinterpret_cast<sockaddr*>(&clientAddr), &nClAddrLen);

		if (clientSocket == INVALID_SOCKET)
		{
			DisplayError("accept()");
			break;
		}

		std::cout << "[TCP 서버] 클라이언트 접속 : IP 주소 = " << inet_ntoa(clientAddr.sin_addr) << ", 포트 번호 = " << ntohs(clientAddr.sin_port) << std::endl;

		while (true)
		{
			int nReturnVal{ recvnc(clientSocket, reinterpret_cast<char*>(&sLen), sizeof(int), 0) };

			if (nReturnVal == SOCKET_ERROR)
			{
				DisplayError("recvn(1)");
				break;
			}
			else if (nReturnVal == 0)
				break;

			sBuf.resize(sLen);

			nReturnVal = recvns(clientSocket, &sBuf, sLen, 0);

			if (nReturnVal == SOCKET_ERROR)
			{
				DisplayError("recvn(2)");
				break;
			}
			else if (nReturnVal == 0)
				break;

			std::cout << "[TCP/" << inet_ntoa(clientAddr.sin_addr) << " : " << ntohs(clientAddr.sin_port) << "] " << sBuf << std::endl;
		}

		closesocket(clientSocket);

		std::cout << "[TCP 서버] 클라이언트 종료 : IP 주소 = " << inet_ntoa(clientAddr.sin_addr) << ", 포트 번호 = " << ntohs(clientAddr.sin_port) << std::endl;
	}

	closesocket(listenSocket);

	WSACleanup();
}

void ErrorQuit(std::string msg)
{
	LPVOID lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPTSTR>(&lpMsgBuf), 0, nullptr);

	MessageBox(nullptr, static_cast<LPCTSTR>(lpMsgBuf), msg.c_str(), MB_ICONERROR);

	LocalFree(lpMsgBuf);
	exit(true);
}

void DisplayError(std::string msg)
{
	LPVOID lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPTSTR>(&lpMsgBuf), 0, nullptr);

	std::cout << "[" << msg << "] " << static_cast<char*>(lpMsgBuf) << std::endl;
	LocalFree(lpMsgBuf);
}

int recvns(SOCKET socket, std::string* buf, int nLength, int flags)
{
	int nReceived;
	int nLeft{ nLength };
	std::string* ptr{ buf };

	while (nLeft > 0)
	{
		nReceived = recv(socket, ptr->data(), nLeft, flags);

		if (nReceived == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (nReceived == 0)
			break;

		nLeft -= nReceived;
		ptr += nReceived;
	}

	return nLength - nLeft;
}

int recvnc(SOCKET socket, char* buf, int nLength, int flags)
{
	int nReceived;
	int nLeft{ nLength };
	char* ptr{ buf };

	while (nLeft > 0)
	{
		nReceived = recv(socket, ptr, nLeft, flags);

		if (nReceived == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (nReceived == 0)
			break;

		nLeft -= nReceived;
		ptr += nReceived;
	}

	return nLength - nLeft;
}