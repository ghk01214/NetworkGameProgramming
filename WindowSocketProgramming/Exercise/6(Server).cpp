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

int clientNum{};			// 연결된 클라이언트 개수
int workingThread{};		// 일하는 스레드 개수

HANDLE hEvent;
bool isSecondThread{ false };

int main()
{
	SOCKET					 listenSocket{ InitSocket() };
	SOCKET					 clientSocket;
	sockaddr_in				 clientAddr;
	int						 cliAddrLen{ sizeof(clientAddr) };
	HANDLE					 hThread;
	std::vector<HANDLE>		 vThread;

	while (true)
	{
		clientSocket = accept(listenSocket, reinterpret_cast<sockaddr*>(&clientAddr), &cliAddrLen);

		if (clientSocket == INVALID_SOCKET)
		{
			DisplayError("accept()");
			break;
		}

		if (!clientNum)
			hEvent = CreateEvent(nullptr, false, true, nullptr);

		++clientNum;

		std::cout << "[TCP 서버] 클라이언트 접속 : IP 주소 = " << inet_ntoa(clientAddr.sin_addr) << ", 포트 번호 = " << ntohs(clientAddr.sin_port) << std::endl << std::endl;

		hThread = CreateThread(nullptr, 0, DownloadData, reinterpret_cast<LPVOID>(clientSocket), 0, nullptr);
		vThread.push_back(hThread);
		
		if (vThread.size() > 1)
			WaitForMultipleObjects(vThread.size(), vThread.data(), true, INFINITE);

		if (hThread == nullptr)
			closesocket(clientSocket);
		else
			CloseHandle(hThread);
	}

	closesocket(listenSocket);
	WSACleanup();
}

// 소켓 함수 오류 출력 후 종료
void ErrorQuit(std::string msg)
{
	LPVOID msgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPTSTR>(&msgBuf), 0, nullptr);

	MessageBox(nullptr, static_cast<LPCTSTR>(msgBuf), msg.c_str(), MB_ICONERROR);

	LocalFree(msgBuf);
	exit(true);
}

// 소켓 함수 오류 출력
void DisplayError(std::string msg)
{
	LPVOID msgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPTSTR>(&msgBuf), 0, nullptr);

	std::cout << "[" << msg << "] " << static_cast<char*>(msgBuf) << std::endl;

	LocalFree(msgBuf);
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
	int				 cliAddrLen{ sizeof(clientAddr) };
	DWORD			 returnVal;

	std::string		 sData;
	std::string		 sLength;
	std::ofstream	 receiveFile;
	std::string		 fileName;

	bool			 getFileSize{ false };
	size_t			 fileSize{};
	size_t			 downloadSize{};
	int				 nLength;

	// 소켓 정보 복사
	getpeername(clientSocket, reinterpret_cast<sockaddr*>(&clientAddr), &cliAddrLen);

	while (true)
	{
		if (clientNum > 1)
			returnVal = WaitForSingleObject(hEvent, INFINITE);

		// 고정 길이 데이터인 가변 길이 데이터의 크기를 받아온다
		returnVal = recvn(clientSocket, &sLength, sizeof(int), 0);

		if (returnVal == SOCKET_ERROR)
		{
			DisplayError("recvn(length)");
			break;
		}
		else if (returnVal == 0)
			break;

		// 고정 길이 데이터로 받은 가변 데이터의 크기에 맞춰 string 크기 변경
		nLength = std::stoi(sLength);
		sData.resize(nLength);

		// 가변 길이 데이터인 실제 전송하고자 하는 데이터를 받아온다
		returnVal = recvn(clientSocket, &sData, nLength, 0);

		if (returnVal == SOCKET_ERROR)
		{
			DisplayError("recvn(data)");
			break;
		}
		else if (returnVal == 0)
			break;

		// 맨 처음 다운 받을 파일의 이름을 sFileName에 저장후 receiveFile 열기
		if (!receiveFile.is_open())
		{
			fileName = sData;
			receiveFile.open(fileName, std::ios::binary);
			++workingThread;

			if (receiveFile.fail())
			{
				std::cout << "파일 생성 실패" << std::endl;
				return 0;
			}
		}
		// 다음으로 받아올 파일의 사이즈를 fileSize에 저장
		else if (!getFileSize)
		{
			getFileSize = true;
			fileSize = std::stoi(sData);		// 수신받은 데이터는 문자열이므로 정수형으로 바꿔준다
		}
		// 이후 파일 데이터 읽어오고 전송률 표시하기
		else
		{
			receiveFile.write(sData.data(), nLength);
			downloadSize += nLength;

			// 소수점 아래 2자리까지 표시되도록 설정
			std::cout.precision(4);
			std::cout << fileName << " 다운로드 중(" << static_cast<float>(downloadSize) / fileSize * 100 << "%)    \n";
			
			if (workingThread == 1)
				std::cout << "\x1B[A";
			else
			{
				if (isSecondThread)
				{
					std::cout << "\x1B[2A";
				}
			}
		}

		if (workingThread > 1)
		{
			if (!isSecondThread)
				isSecondThread = true;
			else
				isSecondThread = false;

			SetEvent(hEvent);
		}
	}

	if (returnVal == 0)
	{
		std::cout << fileName << " 다운로드 완료(" << static_cast<float>(downloadSize) / fileSize * 100 << "%)" << std::endl;
		std::cout << "[TCP/" << inet_ntoa(clientAddr.sin_addr) << " : " << ntohs(clientAddr.sin_port) << "] " << fileName << " 파일 저장" << std::endl << std::endl;
	}

	--clientNum;
	--workingThread;
	SetEvent(hEvent);
	closesocket(clientSocket);
	
	std::cout << "[TCP 서버] 클라이언트 종료 : IP 주소 = " << inet_ntoa(clientAddr.sin_addr) << ", 포트 번호 = " << ntohs(clientAddr.sin_port) << std::endl;
	std::cout << "========================================================================" << std::endl;
}