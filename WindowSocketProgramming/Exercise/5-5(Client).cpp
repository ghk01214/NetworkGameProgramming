#pragma comment(lib, "ws2_32")
#pragma warning(disable : 4996)

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
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

	std::string sName;

	std::cout << "�����ϰ��� �ϴ� ������ �̸� : ";
	std::getline(std::cin, sName);

	std::ifstream fSendFile{ sName, std::ios::binary };

	if (fSendFile.fail())
	{
		std::cout << "���� ���� ����" << std::endl;
		return 0;
	}

	std::vector<std::string> vContain{ sName };
	std::string				 sData;
	int						 sLength;

	fSendFile.seekg(0, std::ios::end);
	sData.resize(fSendFile.tellg());
	fSendFile.seekg(0, std::ios::beg);
	fSendFile.read(sData.data(), sData.size());

	vContain.push_back(sData);

	for (std::string sBytes : vContain)
	{
		sLength = sBytes.length();

		int nReturnVal{ send(connectSocket, reinterpret_cast<char*>(&sLength), sizeof(int), 0) };

		if (nReturnVal == SOCKET_ERROR)
		{
			DisplayError("send(����)");
			break;
		}

		nReturnVal = send(connectSocket, sBytes.data(), sLength, 0);

		if (nReturnVal == SOCKET_ERROR)
		{
			DisplayError("send(������)");
			break;
		}

		std::cout << "[TCP Ŭ���̾�Ʈ] " << sizeof(int) << " + " << nReturnVal << "����Ʈ�� ���½��ϴ�." << std::endl;

		Sleep(1000);
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