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

int nClientNum{};		// 연결된 클라이언트 개수
int nThreadWork{};		// 전체 쓰레드가 일한 횟수

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

		std::cout << "[TCP 서버] 클라이언트 접속 : IP 주소 = " << inet_ntoa(clientAddr.sin_addr) << ", 포트 번호 = " << ntohs(clientAddr.sin_port) << std::endl << std::endl;

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

	// 소켓 정보 복사
	getpeername(clientSocket, reinterpret_cast<sockaddr*>(&clientAddr), &nClAddrLen);

	while (true)
	{
		if (nClientNum > 1)
		{
			nReturnVal = WaitForSingleObject(hClientEvent[my_thread % 2], INFINITE);
		}

		// 고정 길이 데이터인 가변 길이 데이터의 크기를 받아온다
		nReturnVal = recvn(clientSocket, &sLength, sizeof(int), 0);

		if (nReturnVal == SOCKET_ERROR)
		{
			DisplayError("recvn(length)");
			break;
		}
		else if (nReturnVal == 0)
			break;

		// 고정 길이 데이터로 받은 가변 데이터의 크기에 맞춰 string 크기 변경
		nLength = std::stoi(sLength);
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

		// 맨 처음 다운 받을 파일의 이름을 sFileName에 저장후 receiveFile 열기
		if (!receiveFile.is_open())
		{
			sFileName = sData;
			receiveFile.open(sFileName, std::ios::binary);

			if (receiveFile.fail())
			{
				std::cout << "파일 생성 실패" << std::endl;
				return 0;
			}
		}
		// 다음으로 받아올 파일의 사이즈를 fileSize에 저장
		else if (!bGetFileSize)
		{
			bGetFileSize = true;
			fileSize = std::stoi(sData);		// 수신받은 데이터는 문자열이므로 정수형으로 바꿔준다
		}
		// 이후 파일 데이터 읽어오고 전송률 표시하기
		else
		{
			receiveFile.write(sData.data(), nLength);
			downloadSize += nLength;

			// 소수점 아래 2자리까지 표시되도록 설정
			std::cout.precision(3);
			std::cout << sFileName << " 다운로드 중(" << static_cast<float>(downloadSize) / fileSize * 100 << "%)    \n";

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
		std::cout << sFileName << " 다운로드 완료(" << static_cast<float>(downloadSize) / fileSize * 100 << "%)" << std::endl;
		std::cout << "[TCP/" << inet_ntoa(clientAddr.sin_addr) << " : " << ntohs(clientAddr.sin_port) << "] " << sFileName << " 파일 저장" << std::endl << std::endl;
	}

	--nClientNum;
	SetEvent(hClientEvent[my_thread - 1]);
	closesocket(clientSocket);
	receiveFile.close();

	std::cout << "[TCP 서버] 클라이언트 종료 : IP 주소 = " << inet_ntoa(clientAddr.sin_addr) << ", 포트 번호 = " << ntohs(clientAddr.sin_port) << std::endl;
	std::cout << "========================================================================" << std::endl;
}