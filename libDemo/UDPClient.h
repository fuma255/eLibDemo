#include "stdafx.h"
#include "winerror.h"
#include "Winsock2.h"
#pragma comment(lib, "ws2_32")
#include "windows.h"
#include <iostream>
#include <ws2tcpip.h>
#include <stdio.h>

#define UDP_SEND_DATA 10
#define UDP_RECV_DATA 20
#define UDP_ERROR_SOCKET 30
#define BUFFER_SIZE 8192
typedef void(WINAPI *Callback)(int, int, int, char*, int, int);
//�¼�����  ���ݵ�ַ ���ݳ��� �ͻ���ַ �ͻ��˿� ��������
//ȫ�ֺ�������

typedef struct OverlappedP{
	WSAOVERLAPPED overlapped;
	SOCKET socket;
	int OperationType;
	int Callback;
	WSABUF wsabuf;
	char* buf;
	DWORD sendbuflen;/*�ѷ��ͳ���*/
	sockaddr_in addr;
	int Extra;
}OVERLAPPEDP;

void CallSend(OverlappedP* overlapped, DWORD Transferred);
void CallRecv(OverlappedP* over, DWORD Transferred);
DWORD WINAPI Worker(HANDLE iocp);
void destory();
bool InitWinsock();
bool stop(OverlappedP *Recv);
OverlappedP* start(int clientport, int callback, int Extra);
bool sendpack(OverlappedP* recv, char* ip, short port, char* buffer, int buflen);


