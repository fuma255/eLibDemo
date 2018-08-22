#include "stdafx.h"
#include "winerror.h"
#include "Winsock2.h"
#pragma comment(lib, "ws2_32")
#include "windows.h"
#include <iostream>
#include <ws2tcpip.h>
#include <stdio.h>

// 宏定义
#define TCP_SEND_DATA 10
#define TCP_RECV_DATA 20
#define TCP_CLIENT_DISCONNECT 30
#define TCP_SERVER_DISCONNECT 40
#define TCP_EXCEPTION_DISCONNECT 50
#define TCP_ERROR 60
#define TCP_DESTORY 70
#define TCP_BUFFER_SIZE 8192



typedef struct TCPOverlappedP{
	WSAOVERLAPPED overlapped;
	SOCKET socket;
	int OperationType;
	int Callback;
	WSABUF wsabuf;
	char* buf;
	DWORD sendbuflen;/*已发送长度*/
	DWORD wsabuflen;
	sockaddr_in addr;
	int Extra;
	u_short wPort;
}TCPOVERLAPPEDP;

typedef void(WINAPI *TCPCallback)(int, int, int, int, int, int, int);
//事件类型  数据地址 数据长度 服务地址 服务端口 附加数据 错误代码

bool TCPInitWinsock();
TCPOverlappedP* Connect(char* serverIp, int serverPort, int callback, int Extra);
int Send(TCPOverlappedP* recv, char* buf, int len);
int Close(TCPOverlappedP* recv);
void Destory();
u_short GetClientPort(TCPOverlappedP* ptr);


DWORD WINAPI TCPWorker(HANDLE iocp);
void fnRecv(TCPOverlappedP* over, DWORD Transferred);
void fnSend(TCPOverlappedP* overlapped, DWORD Transferred);
void Byte2Hex(const char *sSrc, char *sDest, int nSrcLen);
SOCKET GetSocket(TCPOverlappedP* ptr);

u_short GetServerPort(TCPOverlappedP* ptr);
int GetServerAddr(TCPOverlappedP* ptr);
int GetClientExtra(TCPOverlappedP* ptr);

