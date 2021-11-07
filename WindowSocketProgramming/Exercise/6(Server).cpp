#pragma comment(lib, "ws2_32")
#pragma warning(disable : 4996)

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <WinSock2.h>
#include <Windows.h>

#define SERVERPORT	9000
#define BUFSIZE		512

void ErrorQuit(std::string msg);
void DisplayError(std::string msg);
int recvn(SOCKET socket, std::string* buf, int nLength, int flags);
SOCKET InitSocket();
DWORD WINAPI DownloadData(LPVOID arg);

int nClientNum{};		// ����� Ŭ���̾�Ʈ ����
int nThreadWork{};		// ��ü �����尡 ���� Ƚ��

std::vector<HANDLE> hClientEvent(2);

int main()
{
	SOCKET			 listenSocket{ InitSocket() };
	SOCKET			 clientSocket;
	sockaddr_in		 clientAddr;
	int				 nClAddrLen{ sizeof(clientAddr) };
	std::vector<HANDLE>			 hThread;

	while (true)
	{
		clientSocket = accept(listenSocket, reinterpret_cast<sockaddr*>(&clientAddr), &nClAddrLen);

		if (clientSocket == INVALID_SOCKET)
		{
			DisplayError("accept()");
			break;
		}

		if (!nClientNum)
		{
			hClientEvent[0] = CreateEvent(nullptr, false, true, nullptr);
			hClientEvent[1] = CreateEvent(nullptr, false, false, nullptr);
		}

		++nClientNum;

		std::cout << "[TCP ����] Ŭ���̾�Ʈ ���� : IP �ּ� = " << inet_ntoa(clientAddr.sin_addr) << ", ��Ʈ ��ȣ = " << ntohs(clientAddr.sin_port) << std::endl << std::endl;

		hThread.push_back(CreateThread(nullptr, 0, DownloadData, reinterpret_cast<LPVOID>(clientSocket), 0, nullptr));
		
		if (hThread.size() > 1)
			WaitForMultipleObjects(hThread.size(), hThread.data(), true, INFINITE);

		if (hThread.back() == nullptr)
			closesocket(clientSocket);
		else
		{
			CloseHandle(hThread.back());
			hThread.pop_back();
		}
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

SOCKET InitSocket()
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

	return listenSocket;
}

DWORD WINAPI DownloadData(LPVOID arg)
{
	SOCKET			 clientSocket{ reinterpret_cast<SOCKET>(arg) };
	sockaddr_in		 clientAddr;
	int				 nClAddrLen{ sizeof(clientAddr) };
	DWORD			 nReturnVal;

	std::string		 sData;
	std::string		 sLength;
	std::ofstream	 receiveFile;
	std::string		 sFileName;

	bool			 bGetFileSize{ false };
	size_t			 fileSize{};
	size_t			 downloadSize{};
	int				 nLength;
	int my_thread = nClientNum;

	// ���� ���� ����
	getpeername(clientSocket, reinterpret_cast<sockaddr*>(&clientAddr), &nClAddrLen);

	while (true)
	{
		if (nClientNum > 1)
		{
			nReturnVal = WaitForSingleObject(hClientEvent[my_thread % 2], INFINITE);
		}

		// ���� ���� �������� ���� ���� �������� ũ�⸦ �޾ƿ´�
		nReturnVal = recvn(clientSocket, &sLength, sizeof(int), 0);

		if (nReturnVal == SOCKET_ERROR)
		{
			DisplayError("recvn(length)");
			break;
		}
		else if (nReturnVal == 0)
			break;

		// ���� ���� �����ͷ� ���� ���� �������� ũ�⿡ ���� string ũ�� ����
		nLength = std::stoi(sLength);
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

		// �� ó�� �ٿ� ���� ������ �̸��� sFileName�� ������ receiveFile ����
		if (!receiveFile.is_open())
		{
			sFileName = sData;
			receiveFile.open(sFileName, std::ios::binary);

			if (receiveFile.fail())
			{
				std::cout << "���� ���� ����" << std::endl;
				return 0;
			}
		}
		// �������� �޾ƿ� ������ ����� fileSize�� ����
		else if (!bGetFileSize)
		{
			bGetFileSize = true;
			fileSize = std::stoi(sData);		// ���Ź��� �����ʹ� ���ڿ��̹Ƿ� ���������� �ٲ��ش�
		}
		// ���� ���� ������ �о���� ���۷� ǥ���ϱ�
		else
		{
			receiveFile.write(sData.data(), nLength);
			downloadSize += nLength;

			// �Ҽ��� �Ʒ� 2�ڸ����� ǥ�õǵ��� ����
			std::cout.precision(3);
			std::cout << sFileName << " �ٿ�ε� ��(" << static_cast<float>(downloadSize) / fileSize * 100 << "%)    \n";

			/*if (nClientNum == 1)
				std::cout << "\x1B[A";
			else
			{
				if (my_thread % nClientNum == 0)
				{
					for (int i = 0; i < nClientNum; ++i)
					{
						std::cout << "\x1B[A\x1B[2K";
					}
				}
			}*/
		}

		if (nClientNum > 1)
		{
			SetEvent(hClientEvent[my_thread - 1]);
		}
	}

	if (nReturnVal == 0)
	{
		std::cout << sFileName << " �ٿ�ε� �Ϸ�(" << static_cast<float>(downloadSize) / fileSize * 100 << "%)" << std::endl;
		std::cout << "[TCP/" << inet_ntoa(clientAddr.sin_addr) << " : " << ntohs(clientAddr.sin_port) << "] " << sFileName << " ���� ����" << std::endl << std::endl;
	}

	--nClientNum;
	SetEvent(hClientEvent[my_thread - 1]);
	closesocket(clientSocket);
	receiveFile.close();

	std::cout << "[TCP ����] Ŭ���̾�Ʈ ���� : IP �ּ� = " << inet_ntoa(clientAddr.sin_addr) << ", ��Ʈ ��ȣ = " << ntohs(clientAddr.sin_port) << std::endl;
	std::cout << "========================================================================" << std::endl;
}