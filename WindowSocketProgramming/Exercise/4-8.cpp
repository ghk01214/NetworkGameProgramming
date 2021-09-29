#pragma comment(lib, "ws2_32")
#pragma warning(disable : 4996)

#include <iostream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>

void ErrorQuit(std::string msg);
void DisplayError(std::string msg);

int main()
{
    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != NO_ERROR)
        return 1;

    std::string sIpAddr;
    INT nStartPort, nEndPort;

    std::cout << "IP �ּ� �Է� : ";
    std::getline(std::cin, sIpAddr);
    std::cout << "���� ��Ʈ ��ȣ �Է� : ";
    std::cin >> nStartPort;
    std::cout << "�� ��Ʈ ��ȣ �Է� : ";
    std::cin >> nEndPort;

    INT nPortNum{ nStartPort };

    while (nPortNum <= nEndPort)
    {
        SOCKET connectSocket{ socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) };

        if (connectSocket == SOCKET_ERROR)
            ErrorQuit("socket()");

        sockaddr_in socketAddr;

        ZeroMemory(&socketAddr, sizeof(socketAddr));

        socketAddr.sin_family = AF_INET;
        socketAddr.sin_addr.s_addr = inet_addr(sIpAddr.c_str());
        socketAddr.sin_port = htons(nPortNum);

        if (connect(connectSocket, reinterpret_cast<sockaddr*>(&socketAddr), sizeof(socketAddr)) == SOCKET_ERROR)
            std::cout << nPortNum << "�� ��Ʈ ���� ����" << std::endl;
        else
            std::cout << nPortNum << "�� ��Ʈ�� LISTENING �����Դϴ�." << std::endl;

        ++nPortNum;

        closesocket(connectSocket);
    }

    WSACleanup();
}

void ErrorQuit(std::string msg)
{
    LPVOID lpMsgBuf;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPTSTR>(&lpMsgBuf), 0, nullptr);

    MessageBox(nullptr, static_cast<LPCTSTR>(lpMsgBuf), msg.c_str(), MB_ICONERROR);

    LocalFree(lpMsgBuf);
    exit(TRUE);
}