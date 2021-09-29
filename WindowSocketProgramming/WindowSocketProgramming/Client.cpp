#pragma comment(lib, "ws2_32")
#pragma warning(disable : 4996)

#include <iostream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>

#define SERVERIP4	"127.0.0.1"
#define SERVERIP6	"::1"
#define SERVERPORT	9000
#define BUFFERSIZE	512

void ErrorQuit(std::string msg);
void DisplayError(std::string msg);
INT recvn(SOCKET connectedSocket, std::string buf, INT nLength, INT flags);
DWORD WINAPI TCPClient4();
DWORD WINAPI TCPClient6();

int main()
{
	WSADATA wsa;

	// 윈속 초기화
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != NO_ERROR)
		return 1;

	TCPClient6();

	// 윈속 종료
	WSACleanup();
}

// 소켓 함수 오류 출력 후 종료
void ErrorQuit(std::string msg)
{
	LPVOID lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPTSTR>(&lpMsgBuf), 0, nullptr);

	MessageBox(nullptr, static_cast<LPCTSTR>(lpMsgBuf), msg.c_str(), MB_ICONERROR);

	LocalFree(lpMsgBuf);
	exit(true);
}

// 소켓 함수 오류 출력
void DisplayError(std::string msg)
{
	LPVOID lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPTSTR>(&lpMsgBuf), 0, nullptr);

	std::cout << "[" << msg << "] " << static_cast<char*>(lpMsgBuf) << std::endl;
	LocalFree(lpMsgBuf);
}

INT recvn(SOCKET connectedSocket, std::string buf, INT nLength, INT flags)
{
	INT nReceived;
	std::string ptr{ buf };
	INT nLeft{ nLength };

	while (nLeft > 0)
	{
		nReceived = recv(connectedSocket, ptr.data(), nLeft, flags);

		if (nReceived == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (nReceived == 0)
			break;

		nLeft -= nReceived;
		ptr += nReceived;
	}

	return nLength - nLeft;
}

DWORD WINAPI TCPClient4()
{
	SOCKET sock{ socket(AF_INET, SOCK_STREAM, 0) };

	if (sock == INVALID_SOCKET)
		ErrorQuit("socket()");

	sockaddr_in serverAddr;

	ZeroMemory(&serverAddr, sizeof(serverAddr));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(SERVERIP4);
	serverAddr.sin_port = htons(SERVERPORT);

	if (connect(sock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
		ErrorQuit("connect()");

	std::string buff;

	while (TRUE)
	{
		std::cout << "[보낼 데이터] : ";
		std::getline(std::cin, buff);

		if (buff.length() == 0)
			break;

		INT nReturnVal{ send(sock, buff.data(), buff.length(), 0) };

		if (nReturnVal == SOCKET_ERROR)
		{
			DisplayError("send()");
			break;
		}

		std::cout << "[TCP Client] " << nReturnVal << "바이트를 보냈습니다." << std::endl;

		nReturnVal = recvn(sock, buff, nReturnVal, 0);

		if (nReturnVal == SOCKET_ERROR)
		{
			DisplayError("recv()");
			break;
		}
		else if (nReturnVal == 0)
			break;

		std::cout << "[TCP Client] " << nReturnVal << "바이트를 받았습니다." << std::endl;
		std::cout << "[받은 데이터] : " << buff << std::endl << std::endl;
	}

	return 0;
}

DWORD WINAPI TCPClient6()
{
	SOCKET sock{ socket(AF_INET6, SOCK_STREAM, 0) };

	if (sock == INVALID_SOCKET)
		ErrorQuit("socket()");

	sockaddr_in6 serverAddr;

	ZeroMemory(&serverAddr, sizeof(serverAddr));

	serverAddr.sin6_family = AF_INET6;

	INT nAddrLength{ sizeof(serverAddr) };
	WSAStringToAddress(const_cast<LPSTR>(SERVERIP6), AF_INET6, nullptr, reinterpret_cast<sockaddr*>(&serverAddr), &nAddrLength);

	serverAddr.sin6_port = htons(SERVERPORT);

	if (connect(sock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
		ErrorQuit("connect()");

	std::string buff;

	while (TRUE)
	{
		std::cout << "[보낼 데이터] : ";

		std::getline(std::cin, buff);

		if (buff.length() == 0)
			break;

		INT nReturnVal{ send(sock, buff.data(), buff.length(), 0) };

		if (nReturnVal == SOCKET_ERROR)
		{
			DisplayError("send()");
			break;
		}

		std::cout << "[TCP Client] " << nReturnVal << "바이트를 보냈습니다." << std::endl;

		//nReturnVal = recvn(sock, buf, nReturnVal, 0);

		if (nReturnVal == SOCKET_ERROR)
		{
			DisplayError("recv()");
			break;
		}
		else if (nReturnVal == 0)
			break;

		std::cout << "[TCP Client] " << nReturnVal << "바이트를 받았습니다." << std::endl;
		std::cout << "[받은 데이터] : " << buff << std::endl << std::endl;
	}

	return 0;
}