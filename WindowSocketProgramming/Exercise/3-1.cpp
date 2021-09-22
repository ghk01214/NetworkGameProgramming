#pragma comment(lib, "ws2_32")
#pragma warning(disable : 4996)

#include <iostream>
#include <WinSock2.h>

BOOL IsBigEndian();
BOOL IsLittleEndian();

int main()
{
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) not_eq NO_ERROR)
		return 1;

	if (IsLittleEndian())
		std::cout << "이 시스템은 Little Endian 방식으로 정렬합니다." << std::endl;
	
	if (IsBigEndian())
		std::cout << "이 시스템은 Big Endian 방식으로 정렬합니다." << std::endl;

	WSACleanup();
}

BOOL IsBigEndian()
{
	u_long x{ 0x12345678 };

	return x == htonl(x);
}

BOOL IsLittleEndian()
{
	u_long x{ 0x12345678 };

	return x not_eq htonl(x);
}