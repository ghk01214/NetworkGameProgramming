#pragma comment(lib, "ws2_32")
#pragma warning(disable : 4996)

#include <iostream>
#include <string>
#include <vector>

#include <WinSock2.h>
#include <WS2tcpip.h>

#define SERVERPORT 9000
#define BUFFERSIZE 512

void ErrorQuit(std::string msg);
void DisplayError(std::string msg);
DWORD WINAPI TCPServer4(LPVOID arg);
DWORD WINAPI TCPServer6(LPVOID arg);

int main(int argc, char* argv[])
{
	WSADATA wsa;

	// 윈속 초기화
	if (WSAStartup(MAKEWORD(2, 2), &wsa) not_eq NO_ERROR)
		return 1;

	std::vector<HANDLE> hThread;

	hThread.reserve(2);
	hThread.push_back(CreateThread(nullptr, 0, TCPServer4, nullptr, 0, nullptr));
	hThread.push_back(CreateThread(nullptr, 0, TCPServer6, nullptr, 0, nullptr));

	WaitForMultipleObjects(2, hThread.data(), true, INFINITE);

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
	//exit(true);
}

// 소켓 함수 오류 출력
void DisplayError(std::string msg)
{
	LPVOID lpMsgBuf;

	//FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, WSAGetLastError(),
	//	MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPTSTR>(&lpMsgBuf), 0, nullptr);

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, nullptr);

	std::cout << "[" << msg << "] " << static_cast<char*>(lpMsgBuf) << std::endl;
	LocalFree(lpMsgBuf);
}

// TCP 서버(IPv4)
DWORD WINAPI TCPServer4(LPVOID arg)
{
	int retval;

	// socket()
	// 소켓 타입 : TCP 프로토콜(SOCK_STREAM)
	SOCKET listen_sock{ socket(AF_INET, SOCK_STREAM, 0) };

	if (listen_sock == INVALID_SOCKET)
		ErrorQuit("socket()");
	
	// bind()
	sockaddr_in serveraddr;

	ZeroMemory(&serveraddr, sizeof(serveraddr));

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (sockaddr*)&serveraddr, sizeof(serveraddr));
	// retval = bind(listen_sock, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr));

	if (retval == SOCKET_ERROR)
		ErrorQuit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);

	if (retval == SOCKET_ERROR)
		ErrorQuit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	sockaddr_in clientaddr;
	int addrien;
	char buf[BUFFERSIZE + 1];

	while (true)
	{
		// accept()
		addrien = sizeof(clientaddr);
		client_sock = accept(listen_sock, (sockaddr*)&clientaddr, &addrien);
		//client_sock = accept(listen_sock, reinterpret_cast<sockaddr*>(&clientaddr), &addrien);

		if (client_sock == INVALID_SOCKET)
		{
			DisplayError("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		std::cout << "\n[TCP 서버] 클라이언트 접속 : IP 주소 = " << inet_ntoa(clientaddr.sin_addr) << ", 포트 번호 = " << ntohs(clientaddr.sin_port) << std::endl;

		// 클라이언트와 데이터 통신
		while (true)
		{
			// 데이터 받기
			retval = recv(client_sock, buf, BUFFERSIZE, 0);

			if (retval == SOCKET_ERROR)
			{
				DisplayError("recv()");
				break;
			}
			else if (retval == 0)
				break;

			// 받은 데이터 출력
			buf[retval] = '\0';
			std::cout << buf;

			// 데이터 보내기
			if (send(client_sock, buf, retval, 0) == SOCKET_ERROR)
			{
				DisplayError("send()");
				break;
			}
		}

		// closesocket()
		closesocket(client_sock);
		std::cout << "[TCP 서버] 클라이언트 종료 : IP 주소 = " << inet_ntoa(clientaddr.sin_addr) << ", 포트 번호 = " << ntohs(clientaddr.sin_port) << std::endl;
	}

	// closesocket()
	closesocket(listen_sock);

	return 0;
}

// TCP 서버(IPv6)
DWORD WINAPI TCPServer6(LPVOID arg)
{
	int retval;

	// socket()
	SOCKET listen_sock{ socket(AF_INET6, SOCK_STREAM, 0) };

	if (listen_sock == INVALID_SOCKET)
		ErrorQuit("socket()");

	// bind()
	sockaddr_in6 serveraddr;

	ZeroMemory(&serveraddr, sizeof(serveraddr));

	serveraddr.sin6_family = AF_INET6;
	serveraddr.sin6_addr = in6addr_any;
	serveraddr.sin6_port = htons(SERVERPORT);
	retval = bind(listen_sock, (sockaddr*)&serveraddr, sizeof(serveraddr));
	// retval = bind(listen_sock, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr));

	if (retval == SOCKET_ERROR)
		ErrorQuit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);

	if (retval == SOCKET_ERROR)
		ErrorQuit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	sockaddr_in6 clientaddr;
	int addrien;
	char buf[BUFFERSIZE + 1];

	while (true)
	{
		// accept()
		addrien = sizeof(clientaddr);
		client_sock = accept(listen_sock, (sockaddr*)&clientaddr, &addrien);
		//client_sock = accept(listen_sock, reinterpret_cast<sockaddr*>(&clientaddr), &addrien);

		if (client_sock == INVALID_SOCKET)
		{
			DisplayError("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		char ipaddr[50];
		DWORD ipaddrien{ sizeof(ipaddr) };

		WSAAddressToString((sockaddr*)&clientaddr, sizeof(clientaddr), nullptr, ipaddr, &ipaddrien);
		//WSAAddressToString(reinterpret_cast<sockaddr*>(&clientaddr), sizeof(clientaddr), nullptr, ipaddr, &ipaddrien);
		std::cout << "\n[TCP 서버] 클라이언트 접속 : " << ipaddr << std::endl;

		// 클라이언트와 데이터 통신
		while (true)
		{
			// 데이터 받기
			retval = recv(client_sock, buf, BUFFERSIZE, 0);

			if (retval == SOCKET_ERROR)
			{
				DisplayError("recv()");
				break;
			}
			else if (retval == 0)
				break;

			// 받은 데이터 출력
			buf[retval] = '\0';
			std::cout << buf;

			// 데이터 보내기
			if (send(client_sock, buf, retval, 0) == SOCKET_ERROR)
			{
				DisplayError("send()");
				break;
			}
		}

		// closesocket()
		closesocket(client_sock);
		std::cout << "[TCP 서버] 클라이언트 종료 : " << ipaddr << std::endl;
	}

	// closesocket()
	closesocket(listen_sock);

	return 0;
}