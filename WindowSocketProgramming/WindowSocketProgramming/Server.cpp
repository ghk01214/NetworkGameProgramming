#include "pch.h"

void ErrorQuit(const char* msg);
void DislayError(const char* msg);
DWORD WINAPI TCPServer4(LPVOID arg);
DWORD WINAPI TCPServer6(LPVOID arg);

int main(int argc, char* argv[])
{
	// ���� �ʱ�ȭ
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) not_eq NO_ERROR)
		return 1;

	HANDLE hThread[2];
	hThread[0] = CreateThread(nullptr, 0, TCPServer4, nullptr, 0, nullptr);
	hThread[1] = CreateThread(nullptr, 0, TCPServer6, nullptr, 0, nullptr);

	WaitForMultipleObjects(2, hThread, true, INFINITE);

	// ���� ����
	WSACleanup();
}

// ���� �Լ� ���� ��� �� ����
void ErrorQuit(const char* msg)
{
	LPVOID lpMsgBuf;

	//FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, WSAGetLastError(),
	//	MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), static_cast<LPTSTR>(lpMsgBuf), 0, nullptr);

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, nullptr);
	MessageBox(nullptr, static_cast<LPCTSTR>(lpMsgBuf), msg, MB_ICONERROR);

	LocalFree(lpMsgBuf);
	exit(true);
}

// ���� �Լ� ���� ���
void DislayError(const char* msg)
{
	LPVOID lpMsgBuf;

	//FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, WSAGetLastError(),
	//	MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), static_cast<LPTSTR>(lpMsgBuf), 0, nullptr);

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, nullptr);

	std::cout << "[" << msg << "] " << static_cast<char*>(lpMsgBuf) << std::endl;
	LocalFree(lpMsgBuf);
}

// TCP ����(IPv4)
DWORD WINAPI TCPServer4(LPVOID arg)
{
	int retval;

	// socket()
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
	retval - listen(listen_sock, SOMAXCONN);

	if (retval == SOCKET_ERROR)
		ErrorQuit("listen()");

	// ������ ��ſ� ����� ����
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
			DislayError("accept()");
			break;
		}

		// ������ Ŭ���̾�Ʈ ���� ���
		std::cout << "\n[TCP ����] Ŭ���̾�Ʈ ���� : IP �ּ� = " << inet_ntoa(clientaddr.sin_addr) << ", ��Ʈ ��ȣ = " << ntohs(clientaddr.sin_port) << std::endl;

		// Ŭ���̾�Ʈ�� ������ ���
		while (true)
		{
			// ������ �ޱ�
			retval = recv(client_sock, buf, BUFFERSIZE, 0);

			if (retval == SOCKET_ERROR)
			{
				DislayError("recv()");
				break;
			}
			else if (retval == 0)
				break;

			// ���� ������ ���
			buf[retval] = '\0';
			std::cout << buf;
		}

		// closesocket()
		closesocket(client_sock);
		std::cout << "[TCP ����] Ŭ���̾�Ʈ ���� : IP �ּ� = " << inet_ntoa(clientaddr.sin_addr) << ", ��Ʈ ��ȣ = " << ntohs(clientaddr.sin_port) << std::endl;
	}

	// closesocket()
	closesocket(listen_sock);

	return 0;
}

// TCP ����(IPv6)
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
	retval - listen(listen_sock, SOMAXCONN);

	if (retval == SOCKET_ERROR)
		ErrorQuit("listen()");

	// ������ ��ſ� ����� ����
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
			DislayError("accept()");
			break;
		}

		// ������ Ŭ���̾�Ʈ ���� ���
		char ipaddr[50];
		DWORD ipaddrien{ sizeof(ipaddr) };

		WSAAddressToString((sockaddr*)&clientaddr, sizeof(clientaddr), nullptr, ipaddr, &ipaddrien);
		//WSAAddressToString(reinterpret_cast<sockaddr*>(&clientaddr), sizeof(clientaddr), nullptr, ipaddr, &ipaddrien);
		std::cout << "\n[TCP ����] Ŭ���̾�Ʈ ���� : " << ipaddr << std::endl;

		// Ŭ���̾�Ʈ�� ������ ���
		while (true)
		{
			// ������ �ޱ�
			retval = recv(client_sock, buf, BUFFERSIZE, 0);

			if (retval == SOCKET_ERROR)
			{
				DislayError("recv()");
				break;
			}
			else if (retval == 0)
				break;

			// ���� ������ ���
			buf[retval] = '\0';
			std::cout << buf;
		}

		// closesocket()
		closesocket(client_sock);
		std::cout << "[TCP ����] Ŭ���̾�Ʈ ���� : " << ipaddr << std::endl;
	}

	// closesocket()
	closesocket(listen_sock);

	return 0;
}