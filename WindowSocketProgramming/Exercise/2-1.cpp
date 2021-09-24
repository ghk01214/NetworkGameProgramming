#pragma comment(lib, "ws2_32")
#pragma warning(disable : 4996)

#include <iostream>
#include <WinSock2.h>

int main()
{
	WSADATA wsa2;
	WSADATA wsa1;

	// ������ ���� 2.2 Ver. �ʱ�ȭ
	if (WSAStartup(MAKEWORD(2, 2), &wsa2) not_eq NO_ERROR)
		return 1;

	// ������ ���� 1.1 Ver. �ʱ�ȭ
	if (WSAStartup(MAKEWORD(1, 1), &wsa1) not_eq NO_ERROR)
		return 1;

	// wVersion			 = WS2_32.dll���� �ε�� ������ ������ ����
	// wHighVersion		 = WS2_32.dll�� �����ϴ� ������ ������ �ֻ��� ����(�Ϲ������� wVersion ���ڿ� ����)
	// szDescription	 = WS2_32.dll�� ������ ���� ������ ���� ������ �����ϴ� NULL�� ������ ASCII ���ڿ�
	// szSystemStatus	 = WS2_32.dll�� ������ ������ ���� ���¿� ���� ������ �����ϴ� NULL�� ������ ASCII ���ڿ�
	std::cout << "Window Socket 2.2" << std::endl;
	std::cout << (wsa2.wVersion & 0xff) << "." << (wsa2.wVersion >> 8) << ", " << (wsa2.wHighVersion & 0xff) << "." << (wsa2.wHighVersion >> 8)
		<< ", " << wsa2.szDescription << ", " << wsa2.szSystemStatus << std::endl << std::endl;

	std::cout << "Window Socket 1.1" << std::endl;
	std::cout << (wsa1.wVersion & 0xff) << "." << (wsa1.wVersion >> 8) << ", " << (wsa1.wHighVersion & 0xff) << "." << (wsa1.wHighVersion >> 8)
		<< ", " << wsa1.szDescription << ", " << wsa1.szSystemStatus << std::endl << std::endl;

	MessageBox(nullptr, L"������ ���� �ʱ�ȭ ����", L"�˸�", MB_OK);
	
	// ������ ���� ����
	WSACleanup();
}