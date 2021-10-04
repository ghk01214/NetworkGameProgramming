#pragma comment(lib, "ws2_32")
#pragma warning(disable : 4996)

#include <iostream>
#include <string>
#include <vector>
#include <WinSock2.h>
#include <WS2tcpip.h>

#define SERVERIP	"127.0.0.1"
#define SERVERPORT	9000
#define BUFSIZE		50

void ErrorQuit(std::string msg);
void DisplayError(std::string msg);

int main()
{
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != NOERROR)
		return 1;

	SOCKET connectSocket{ socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) };

	if (connectSocket == INVALID_SOCKET)
		ErrorQuit("socket()");

	sockaddr_in serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serverAddr.sin_port = htons(SERVERPORT);

	if (connect(connectSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
		ErrorQuit("connect()");

	std::vector<std::string> vTestData{
		"A : 안녕하세요.",
		"B : 반가워요.",
		"A : 오늘따라 할 이야기가 많을 것 같네요.",
		"B : 저도 그렇네요."
	};

	for (std::string str : vTestData)
	{
		std::string buf(BUFSIZE, '#');
		str.copy(buf.data(), str.length());

		int nReturnVal = send(connectSocket, buf.data(), BUFSIZE, 0);

		if (nReturnVal == SOCKET_ERROR)
		{
			DisplayError("send(2)");
			break;
		}

		std::cout << "[TCP 클라이언트] " << nReturnVal <<  " + " << sizeof(int) << "바이트를 보냈습니다." << std::endl;
	}

	closesocket(connectSocket);

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