#pragma once
#pragma comment(lib, "ws2_32")
#pragma warning(disable : 4996)

#include <iostream>
#include <string>
#include <vector>

#include <WinSock2.h>
#include <WS2tcpip.h>

#define SERVERPORT 9000
#define BUFFERSIZE 512