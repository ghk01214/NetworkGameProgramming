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

    std::cout << "IP 주소 입력 : ";
    std::getline(std::cin, sIpAddr);
    std::cout << "시작 포트 번호 입력 : ";
    std::cin >> nStartPort;
    std::cout << "끝 포트 번호 입력 : ";
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
            std::cout << nPortNum << "번 포트 연결 실패" << std::endl;
        else
            std::cout << nPortNum << "번 포트가 LISTENING 상태입니다." << std::endl;

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