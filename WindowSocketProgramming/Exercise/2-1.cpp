#pragma comment(lib, "ws2_32")
#pragma warning(disable : 4996)

#include <iostream>
#include <WinSock2.h>

int main()
{
	WSADATA wsa2;
	WSADATA wsa1;

	// 윈도우 소켓 2.2 Ver. 초기화
	if (WSAStartup(MAKEWORD(2, 2), &wsa2) not_eq NO_ERROR)
		return 1;

	// 윈도우 소켓 1.1 Ver. 초기화
	if (WSAStartup(MAKEWORD(1, 1), &wsa1) not_eq NO_ERROR)
		return 1;

	// wVersion			 = WS2_32.dll에서 로드된 윈도우 소켓의 버전
	// wHighVersion		 = WS2_32.dll이 지원하는 윈도우 소켓의 최상위 버전(일반적으로 wVersion 인자와 동일)
	// szDescription	 = WS2_32.dll이 윈도우 소켓 구현에 대한 설명을 복사하는 NULL로 끝나는 ASCII 문자열
	// szSystemStatus	 = WS2_32.dll이 윈도우 소켓의 관련 상태와 구성 정보를 복사하는 NULL로 끝나는 ASCII 문자열
	std::cout << "Window Socket 2.2" << std::endl;
	std::cout << (wsa2.wVersion & 0xff) << "." << (wsa2.wVersion >> 8) << ", " << (wsa2.wHighVersion & 0xff) << "." << (wsa2.wHighVersion >> 8)
		<< ", " << wsa2.szDescription << ", " << wsa2.szSystemStatus << std::endl << std::endl;

	std::cout << "Window Socket 1.1" << std::endl;
	std::cout << (wsa1.wVersion & 0xff) << "." << (wsa1.wVersion >> 8) << ", " << (wsa1.wHighVersion & 0xff) << "." << (wsa1.wHighVersion >> 8)
		<< ", " << wsa1.szDescription << ", " << wsa1.szSystemStatus << std::endl << std::endl;

	MessageBox(nullptr, L"윈도우 소켓 초기화 성공", L"알림", MB_OK);
	
	// 윈도우 소켓 종료
	WSACleanup();
}