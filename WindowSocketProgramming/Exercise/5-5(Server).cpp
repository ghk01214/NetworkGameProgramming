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

		std::cout << "[TCP 서버] 클라이언트 접속 : IP 주소 = " << inet_ntoa(clientAddr.sin_addr) << ", 포트 번호 = " << ntohs(clientAddr.sin_port) << std::endl << std::endl;

		bool	 bGetFileSize{ false };
		size_t	 fileSize{};
		size_t	 downloadSize{};
		int		 nLength;

		while (true)
		{
			// 고정 길이 데이터인 가변 길이 데이터의 크기를 받아온다
			int nReturnVal{ recvn(clientSocket, &sLength, sizeof(int), 0) };

			if (nReturnVal == SOCKET_ERROR)
			{
				DisplayError("recvn(length)");
				break;
			}
			else if (nReturnVal == 0)
				break;

			nLength = std::stoi(sLength);

			// 고정 길이 데이터로 받은 가변 데이터의 크기에 맞춰 string 크기 변경
			sData.resize(nLength);

			// 가변 길이 데이터인 실제 전송하고자 하는 데이터를 받아온다
			nReturnVal = recvn(clientSocket, &sData, nLength, 0);

			if (nReturnVal == SOCKET_ERROR)
			{
				DisplayError("recvn(data)");
				break;
			}
			else if (nReturnVal == 0)
				break;

			// 맨 처음에 받아올 파일의 사이즈를 fileSize에 저장
			if (!bGetFileSize)
			{
				bGetFileSize = true;
				fileSize = std::stoi(sData);		// 수신받은 데이터는 문자열이므로 정수형으로 바꿔준다
			}
			// 다음으로 다운 받을 파일의 이름을 sFileName에 저장후 receiveFile 열기
			else if (!receiveFile.is_open())
			{
				sFileName = sData;
				receiveFile.open(sFileName, std::ios::binary);

				if (receiveFile.fail())
				{
					std::cout << "파일 생성 실패" << std::endl;
					return 0;
				}
			}
			// 이후 파일 데이터 읽어오고 전송률 표시하기
			else
			{
				receiveFile.write(sData.data(), nLength);
				downloadSize += nLength;

				// 소수점 아래 2자리까지 표시되도록 설정
				std::cout.precision(4);
				std::cout << "다운로드 중(" << static_cast<float>(downloadSize) / fileSize * 100 << "%)    \r";
			}
		}

		std::cout << "다운로드 완료(" << static_cast<float>(downloadSize) / fileSize * 100 << "%)" << std::endl;
		std::cout << "[TCP/" << inet_ntoa(clientAddr.sin_addr) << " : " << ntohs(clientAddr.sin_port) << "] " << sFileName << " 파일 저장" << std::endl << std::endl;

		closesocket(clientSocket);
		receiveFile.close();

		std::cout << "[TCP 서버] 클라이언트 종료 : IP 주소 = " << inet_ntoa(clientAddr.sin_addr) << ", 포트 번호 = " << ntohs(clientAddr.sin_port) << std::endl;
		std::cout << "========================================================================" << std::endl;
	}

	closesocket(listenSocket);

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