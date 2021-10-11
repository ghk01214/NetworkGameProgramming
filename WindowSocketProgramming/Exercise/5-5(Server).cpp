#pragma comment(lib, "ws2_32")
#pragma warning(disable : 4996)

#include <iostream>
#include <string>
#include <fstream>
#include <WinSock2.h>
#include <WS2tcpip.h>

#define SERVERPORT	9000
#define BUFSIZE		512

void ErrorQuit(std::string msg);
void DisplayError(std::string msg);
int recvn(SOCKET socket, std::string* buf, int nLength, int flags);

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

	SOCKET			 clientSocket;
	sockaddr_in		 clientAddr;
	int				 nClAddrLen;

	std::string		 sData;
	std::string		 sLength;

	std::ofstream	 receiveFile;
	std::string		 sFileName;

	while (true)
	{
		nClAddrLen = sizeof(clientAddr);
		clientSocket = accept(listenSocket, reinterpret_cast<sockaddr*>(&clientAddr), &nClAddrLen);

		if (clientSocket == INVALID_SOCKET)
		{
			DisplayError("accept()");
			break;
		}

		std::cout << "[TCP ����] Ŭ���̾�Ʈ ���� : IP �ּ� = " << inet_ntoa(clientAddr.sin_addr) << ", ��Ʈ ��ȣ = " << ntohs(clientAddr.sin_port) << std::endl << std::endl;

		bool	 bGetFileSize{ false };
		size_t	 fileSize{};
		size_t	 downloadSize{};
		int		 nLength;

		while (true)
		{
			// ���� ���� �������� ���� ���� �������� ũ�⸦ �޾ƿ´�
			int nReturnVal{ recvn(clientSocket, &sLength, sizeof(int), 0) };

			if (nReturnVal == SOCKET_ERROR)
			{
				DisplayError("recvn(length)");
				break;
			}
			else if (nReturnVal == 0)
				break;

			nLength = std::stoi(sLength);

			// ���� ���� �����ͷ� ���� ���� �������� ũ�⿡ ���� string ũ�� ����
			sData.resize(nLength);

			// ���� ���� �������� ���� �����ϰ��� �ϴ� �����͸� �޾ƿ´�
			nReturnVal = recvn(clientSocket, &sData, nLength, 0);

			if (nReturnVal == SOCKET_ERROR)
			{
				DisplayError("recvn(data)");
				break;
			}
			else if (nReturnVal == 0)
				break;

			// �� ó���� �޾ƿ� ������ ����� fileSize�� ����
			if (!bGetFileSize)
			{
				bGetFileSize = true;
				fileSize = std::stoi(sData);		// ���Ź��� �����ʹ� ���ڿ��̹Ƿ� ���������� �ٲ��ش�
			}
			// �������� �ٿ� ���� ������ �̸��� sFileName�� ������ receiveFile ����
			else if (!receiveFile.is_open())
			{
				sFileName = sData;
				receiveFile.open(sFileName, std::ios::binary);

				if (receiveFile.fail())
				{
					std::cout << "���� ���� ����" << std::endl;
					return 0;
				}
			}
			// ���� ���� ������ �о���� ���۷� ǥ���ϱ�
			else
			{
				receiveFile.write(sData.data(), nLength);
				downloadSize += nLength;

				// �Ҽ��� �Ʒ� 2�ڸ����� ǥ�õǵ��� ����
				std::cout.precision(4);
				std::cout << "�ٿ�ε� ��(" << static_cast<float>(downloadSize) / fileSize * 100 << "%)    \r";
			}
		}

		std::cout << "�ٿ�ε� �Ϸ�(" << static_cast<float>(downloadSize) / fileSize * 100 << "%)" << std::endl;
		std::cout << "[TCP/" << inet_ntoa(clientAddr.sin_addr) << " : " << ntohs(clientAddr.sin_port) << "] " << sFileName << " ���� ����" << std::endl << std::endl;

		closesocket(clientSocket);
		receiveFile.close();

		std::cout << "[TCP ����] Ŭ���̾�Ʈ ���� : IP �ּ� = " << inet_ntoa(clientAddr.sin_addr) << ", ��Ʈ ��ȣ = " << ntohs(clientAddr.sin_port) << std::endl;
		std::cout << "========================================================================" << std::endl;
	}

	closesocket(listenSocket);

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

	std::cout << "[" << msg << "] " << static_cast<char*>(lpMsgBuf) << std::endl;

	LocalFree(lpMsgBuf);
}

int recvn(SOCKET socket, std::string* buf, int nLength, int flags)
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