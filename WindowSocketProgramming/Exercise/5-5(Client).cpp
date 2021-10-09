#pragma comment(lib, "ws2_32")
#pragma warning(disable : 4996)

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <random>
#include <WinSock2.h>
#include <WS2tcpip.h>

#define SERVERIP	"127.0.0.1"
#define SERVERPORT	9000
#define MAXBUFSIZE	512
#define MINBUFSIZE	50

std::random_device rd;
std::default_random_engine dre(rd());

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

	std::string sFileName;

	std::cout << "�����ϰ��� �ϴ� ������ �̸� : ";
	std::getline(std::cin, sFileName);

	std::ifstream fSendFile{ sFileName, std::ios::binary };

	if (fSendFile.fail())
	{
		std::cout << "���� �б� ����" << std::endl;
		return 0;
	}

	std::vector<std::string> vContain;
	std::string				 sData;
	int						 nLength;
	size_t					 fileSize;

	// ������ ũ�� Ȯ��
	fSendFile.seekg(0, std::ios::end);
	fileSize = fSendFile.tellg();
	fSendFile.seekg(0, std::ios::beg);

	// ������ ������ ����� ���� vector�� push�� ���� ���� �̸��� push�Ѵ�
	// ���� ���� : ���� ũ�� -> ���� �̸� -> ���� ������
	vContain.push_back(std::to_string(fileSize));
	vContain.push_back(sFileName);

	// �����ϴ� �����͸� ���� ���̷� �ϱ� ���� �Ź� �����ϴ� ����Ʈ ���� 50~ 512 ����Ʈ ���̿��� �������� ����
	std::uniform_int_distribution<>	 uid(MINBUFSIZE, MAXBUFSIZE);
	size_t							 uploadSize{};

	while (true)
	{
		// ���� ������ ���� ���� ����
		int randSize{ uid(dre) };
		uploadSize += randSize;
		
		// ������ �� ����Ʈ ���� ������ ũ�⺸�� Ŭ ��� ���� ����� ���缭 ���� ����Ʈ �뷮�� �����
		if (uploadSize > fileSize)
		{
			uploadSize -= randSize;
			randSize = fileSize - uploadSize;
			uploadSize += randSize;

			sData.resize(randSize);
			fSendFile.read(sData.data(), sData.size());
			vContain.push_back(sData);

			break;
		}

		sData.resize(randSize);
		fSendFile.read(sData.data(), sData.size());
		vContain.push_back(sData);
	}

	uploadSize = 0;

	for (std::string sBytes : vContain)
	{
		nLength = sBytes.length();

		int nReturnVal{ send(connectSocket, reinterpret_cast<std::string*>(&nLength)->data(), sizeof(int), 0) };
		//int nReturnVal{ send(connectSocket, std::to_string(sLength).data(), sizeof(int), 0) };

		if (nReturnVal == SOCKET_ERROR)
		{
			DisplayError("send(����)");
			break;
		}

		nReturnVal = send(connectSocket, sBytes.data(), nLength, 0);

		if (nReturnVal == SOCKET_ERROR)
		{
			DisplayError("send(������)");
			break;
		}

		uploadSize += nLength;

		std::cout.precision(4);
		std::cout << "\x1B[2K";
		std::cout << "[TCP Ŭ���̾�Ʈ] " << sizeof(int) << " + " << nReturnVal << "����Ʈ�� ���½��ϴ�." << std::endl;
		std::cout << "���ε� ��(" << static_cast<float>(uploadSize) / fileSize * 100 << "%)   " << std::endl;
		std::cout << "\x1B[2A";
	}

	std::cout << "\x1B[B\x1B[K\x1B[A\x1B[K";
	std::cout << "���ε� �Ϸ�(" << static_cast<float>(uploadSize) / fileSize * 100 << "%, �� " << fileSize << "����Ʈ)" << std::endl;

	closesocket(connectSocket);

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