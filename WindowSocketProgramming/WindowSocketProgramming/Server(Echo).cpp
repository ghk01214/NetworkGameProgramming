#pragma comment(lib, "ws2_32")
#pragma warning(disable : 4996)

#include <iostream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>

#define SERVERPORT 9000
#define BUFFERSIZE 512

void ErrorQuit(std::string msg);
void DisplayError(std::string msg);
DWORD WINAPI TCPServer4();
DWORD WINAPI TCPServer6();
DWORD WINAPI ProcessClient(LPVOID arg);

int main()
{
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != NO_ERROR)
		return 1;

	TCPServer4();

	WSACleanup();
}

// ���� �Լ� ���� ��� �� ����
void ErrorQuit(std::string msg)
{
	LPVOID lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPTSTR>(&lpMsgBuf), 0, nullptr);

	MessageBox(nullptr, static_cast<LPCTSTR>(lpMsgBuf), msg.c_str(), MB_ICONERROR);

	LocalFree(lpMsgBuf);
	exit(true);
}

// ���� �Լ� ���� ���
void DisplayError(std::string msg)
{
	LPVOID lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPTSTR>(&lpMsgBuf), 0, nullptr);

	std::cout << "[" << msg << "] " << static_cast<CHAR*>(lpMsgBuf) << std::endl;
	LocalFree(lpMsgBuf);
}

// TCP ����(IPv4)
DWORD WINAPI TCPServer4()
{
	// ���� Ÿ�� : TCP ��������(SOCK_STREAM)
	SOCKET listenSocket{ socket(AF_INET, SOCK_STREAM, 0) };

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
	INT			 nAddrLen{ sizeof(clientAddr) };
	HANDLE		 hThread;
	std::string	 buff;
	buff.resize(BUFFERSIZE);

	while (true)
	{
		clientSocket = accept(listenSocket, reinterpret_cast<sockaddr*>(&clientAddr), &nAddrLen);

		if (clientSocket == INVALID_SOCKET)
		{
			DisplayError("accept()");
			break;
		}

		std::cout << "[TCP ����] Ŭ���̾�Ʈ ���� : IP �ּ� = " << inet_ntoa(clientAddr.sin_addr) << ", ��Ʈ ��ȣ = " << ntohs(clientAddr.sin_port) << std::endl;

		hThread = CreateThread(nullptr, 0, ProcessClient, reinterpret_cast<LPVOID>(clientSocket), 0, nullptr);

		if (hThread == nullptr)
			closesocket(clientSocket);
		else
			CloseHandle(hThread);
	}

	closesocket(listenSocket);

	return 0;
}

// TCP ����(IPv6)
DWORD WINAPI TCPServer6()
{
	SOCKET listen_sock{ socket(AF_INET6, SOCK_STREAM, 0) };

	if (listen_sock == INVALID_SOCKET)
		ErrorQuit("socket()");

	sockaddr_in6 serveraddr;

	ZeroMemory(&serveraddr, sizeof(serveraddr));

	serveraddr.sin6_family = AF_INET6;
	serveraddr.sin6_addr = in6addr_any;
	serveraddr.sin6_port = htons(SERVERPORT);

	if (bind(listen_sock, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR)
		ErrorQuit("bind()");

	if (listen(listen_sock, SOMAXCONN) == SOCKET_ERROR)
		ErrorQuit("listen()");

	SOCKET		 clientSocket;
	sockaddr_in6 clientAddr;
	INT			 nAddrLen;
	std::string	 buff;
	buff.resize(BUFFERSIZE + 1);

	while (TRUE)
	{
		nAddrLen = sizeof(clientAddr);
		clientSocket = accept(listen_sock, reinterpret_cast<sockaddr*>(&clientAddr), &nAddrLen);

		if (clientSocket == INVALID_SOCKET)
		{
			DisplayError("accept()");
			break;
		}

		std::string sIpAddr;
		sIpAddr.resize(50);
		DWORD dwIpAddrLen{ sizeof(sIpAddr) };

		WSAAddressToString(reinterpret_cast<sockaddr*>(&clientAddr), sizeof(clientAddr), nullptr, sIpAddr.data(), &dwIpAddrLen);

		std::cout << "[TCP ����] Ŭ���̾�Ʈ ���� : " << sIpAddr << std::endl;

		// Ŭ���̾�Ʈ�� ������ ���
		while (TRUE)
		{
			INT nReturnVal{ recv(clientSocket, buff.data(), BUFFERSIZE, 0) };

			if (nReturnVal == SOCKET_ERROR)
			{
				DisplayError("recv()");
				break;
			}
			else if (nReturnVal == 0)
				break;

			// ���� ������ ���
			std::cout << buff << std::endl;

			// ������ ������
			if (send(clientSocket, buff.data(), nReturnVal, 0) == SOCKET_ERROR)
			{
				DisplayError("send()");
				break;
			}
		}

		closesocket(clientSocket);
		std::cout << "[TCP ����] Ŭ���̾�Ʈ ���� : " << sIpAddr << std::endl;
	}

	closesocket(listen_sock);

	return 0;
}

DWORD WINAPI ProcessClient(LPVOID arg)
{
	SOCKET clientSocket{ reinterpret_cast<SOCKET>(arg) };
	sockaddr_in clientAddr;
	int nAddrLen{ sizeof(clientAddr) };
	std::string sBuf;

	getpeername(clientSocket, reinterpret_cast<sockaddr*>(&clientAddr), &nAddrLen);

	while (true)
	{
		sBuf.resize(BUFFERSIZE);
		int nReturnVal{ recv(clientSocket, sBuf.data(), BUFFERSIZE, 0) };

		if (nReturnVal == SOCKET_ERROR)
		{
			DisplayError("recv()");
			break;
		}
		else if (nReturnVal == 0)
			break;

		sBuf.resize(nReturnVal);

		std::cout << "[TCP/" << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << "] " << sBuf << std::endl;

		nReturnVal = send(clientSocket, sBuf.data(), nReturnVal, 0);

		if (nReturnVal == SOCKET_ERROR)
		{
			DisplayError("send()");
			break;
		}
	}

	closesocket(clientSocket);

	std::cout << "[TCP ����] Ŭ���̾�Ʈ ���� : IP �ּ� = " << inet_ntoa(clientAddr.sin_addr) << ", ��Ʈ ��ȣ = " << ntohs(clientAddr.sin_port) << std::endl;

	return 0;
}