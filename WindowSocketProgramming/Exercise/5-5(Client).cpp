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

	std::cout << "전송하고자 하는 파일의 이름 : ";
	std::getline(std::cin, sFileName);

	std::ifstream fSendFile{ sFileName, std::ios::binary };

	if (fSendFile.fail())
	{
		std::cout << "파일 읽기 실패" << std::endl;
		return 0;
	}

	std::vector<std::string> vContain;
	std::string				 sData;
	int						 nLength;
	size_t					 fileSize;

	// 파일의 크기 확인
	fSendFile.seekg(0, std::ios::end);
	fileSize = fSendFile.tellg();
	fSendFile.seekg(0, std::ios::beg);

	// 전송할 파일의 사이즈를 먼저 vector에 push한 다음 파일 이름을 push한다
	// 전송 순서 : 파일 크기 -> 파일 이름 -> 파일 데이터
	vContain.push_back(std::to_string(fileSize));
	vContain.push_back(sFileName);

	// 전송하는 데이터를 가변 길이로 하기 위해 매번 전송하는 바이트 수를 50~ 512 바이트 사이에서 랜덤으로 설정
	std::uniform_int_distribution<>	 uid(MINBUFSIZE, MAXBUFSIZE);
	size_t							 uploadSize{};

	while (true)
	{
		// 가변 데이터 길이 랜덤 설정
		int randSize{ uid(dre) };
		uploadSize += randSize;
		
		// 전송한 총 바이트 수가 파일의 크기보다 클 경우 파일 사이즈에 맞춰서 전송 바이트 용량을 맞춘다
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
			DisplayError("send(길이)");
			break;
		}

		nReturnVal = send(connectSocket, sBytes.data(), nLength, 0);

		if (nReturnVal == SOCKET_ERROR)
		{
			DisplayError("send(데이터)");
			break;
		}

		uploadSize += nLength;

		std::cout.precision(4);
		std::cout << "\x1B[2K";
		std::cout << "[TCP 클라이언트] " << sizeof(int) << " + " << nReturnVal << "바이트를 보냈습니다." << std::endl;
		std::cout << "업로드 중(" << static_cast<float>(uploadSize) / fileSize * 100 << "%)   " << std::endl;
		std::cout << "\x1B[2A";
	}

	std::cout << "\x1B[B\x1B[K\x1B[A\x1B[K";
	std::cout << "업로드 완료(" << static_cast<float>(uploadSize) / fileSize * 100 << "%, 총 " << fileSize << "바이트)" << std::endl;

	closesocket(connectSocket);

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