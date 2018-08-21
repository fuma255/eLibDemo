#include "stdafx.h"
#include "UDPClient.h"


HANDLE hIocp;

DWORD WINAPI Worker(HANDLE iocp){
	DWORD Transferred;
	SOCKET lpCompletekey;//PULONG_PTR
	OverlappedP* lpOverlapped;//LPOVERLAPPED
	unsigned long iFlag = 0;
	int ret = 0;
	int fromLen = 16;
	while (true){
		if (GetQueuedCompletionStatus(iocp, &Transferred, (LPDWORD)&lpCompletekey, (LPOVERLAPPED *)&lpOverlapped, 500)){
			if (lpOverlapped == NULL){
				continue;
			}
			switch (lpOverlapped->OperationType){
			case UDP_RECV_DATA:
				if (lpOverlapped->Callback != 0){
					Callback funCall = (Callback)lpOverlapped->Callback;
					funCall(lpOverlapped->OperationType, (int)lpOverlapped->wsabuf.buf, Transferred, inet_ntoa(lpOverlapped->addr.sin_addr), ntohs(lpOverlapped->addr.sin_port), lpOverlapped->Extra);
				}
				//Sleep(100);
				ret = WSARecvFrom(lpOverlapped->socket, &lpOverlapped->wsabuf, 1, &lpOverlapped->sendbuflen, &iFlag, (sockaddr *)&lpOverlapped->addr, &fromLen, &lpOverlapped->overlapped, 0);
				if (ret == SOCKET_ERROR){
					ret = WSAGetLastError();
					//#define ERROR_INVALID_HANDLE 6L //The handle is invalid.
					if (ret != WSA_IO_PENDING){
						printf("WSARecvFrom SOCKET_ERROR!");
					}

				}
				//CallRecv(lpOverlapped, Transferred);
				break;
			case UDP_SEND_DATA:
				CallSend(lpOverlapped, Transferred);
				break;
			case UDP_ERROR_SOCKET:
				break;
			default:
				break;
			}
		}
	}
	return 0;
}

void CallRecv(OverlappedP* over, DWORD Transferred){
	//FILE* pFile = fopen("C:\\Users\\Administrator\\Desktop\\fnesample\\123.txt", "a");
	//time_t time_log = time(NULL);
	//struct tm* tm_log = localtime(&time_log);
	//char chs[1024];
	//fprintf(pFile, "[%04d-%02d-%02d %02d:%02d:%02d] -> i=%d\n", tm_log->tm_year + 1900, tm_log->tm_mon + 1, tm_log->tm_mday, tm_log->tm_hour, tm_log->tm_min, tm_log->tm_sec, 1);

	unsigned long iFlag = 0;
	int ret = 0;
	if (over->Callback != 0){
		Callback funCall = (Callback)over->Callback;
		funCall(over->OperationType, (int)over->wsabuf.buf, Transferred, inet_ntoa(over->addr.sin_addr), ntohs(over->addr.sin_port), over->Extra);
	}
	//fprintf(pFile, "[%04d-%02d-%02d %02d:%02d:%02d] -> i=%d\n", tm_log->tm_year + 1900, tm_log->tm_mon + 1, tm_log->tm_mday, tm_log->tm_hour, tm_log->tm_min, tm_log->tm_sec, 2);
	int fromLen = 16;
	ret = WSARecvFrom(over->socket, &over->wsabuf, 1, &over->sendbuflen, &iFlag, (sockaddr *)&over->addr, &fromLen, &over->overlapped, 0);
	//fprintf(pFile, "[%04d-%02d-%02d %02d:%02d:%02d] -> i=%d\n", tm_log->tm_year + 1900, tm_log->tm_mon + 1, tm_log->tm_mday, tm_log->tm_hour, tm_log->tm_min, tm_log->tm_sec, 3);
	if (ret == SOCKET_ERROR){
		ret = WSAGetLastError();
		//#define ERROR_INVALID_HANDLE 6L //The handle is invalid.
		if (ret != WSA_IO_PENDING){
			printf("WSARecvFrom SOCKET_ERROR!");
		}
		
	}
	/*
	fprintf(pFile, "[%04d-%02d-%02d %02d:%02d:%02d] -> ret=%d\n", tm_log->tm_year + 1900, tm_log->tm_mon + 1, tm_log->tm_mday, tm_log->tm_hour, tm_log->tm_min, tm_log->tm_sec, ret);
	fflush(pFile);
	fclose(pFile);
	*/
}

void CallSend(OverlappedP* overlapped, DWORD Transferred){



	overlapped->sendbuflen += Transferred;

	/*1:Transferred=1024  overlapped->sendbuflen=1024 overlapped->wsabuf.len=4096-1024     overlapped->wsabuf.buf=overlapped->wsabuf.buf-1024
	2:Transferred=1024  overlapped->sendbuflen=2048 overlapped->wsabuf.len=4096-2048     overlapped->wsabuf.buf=overlapped->wsabuf.buf-2048
	3:Transferred=1024  overlapped->sendbuflen=3072 overlapped->wsabuf.len=4096-3072     overlapped->wsabuf.buf=overlapped->wsabuf.buf-3072
	*/

	if (overlapped->sendbuflen < overlapped->wsabuf.len){
		overlapped->wsabuf.len -= overlapped->sendbuflen;
		overlapped->wsabuf.buf += overlapped->sendbuflen;
		WSASendTo(overlapped->socket, &overlapped->wsabuf, 1, NULL, 0, (sockaddr *)&overlapped->addr, sizeof(sockaddr_in), &overlapped->overlapped, 0);
	}
	else{
		Callback funCall = (Callback)overlapped->Callback;
		funCall(overlapped->OperationType, (int)overlapped->wsabuf.buf, Transferred, inet_ntoa(overlapped->addr.sin_addr), ntohs(overlapped->addr.sin_port), overlapped->Extra);
		free(overlapped->buf);
		free(overlapped);
	}

}


void destory(){
	if (hIocp > 0){
		PostQueuedCompletionStatus(hIocp, 0, 0, 0);
		CloseHandle(hIocp);
	}
}

bool InitWinsock(){

	SYSTEM_INFO system;

	DWORD ThreadID;

	if ((hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL){
		printf("CreateIoCompletionPort failed!\n");
		return false;
	}
	GetSystemInfo(&system);
	int ThreadCount = system.dwNumberOfProcessors * 2;
	for (int i = 0; i < ThreadCount; i++){
		HANDLE ThreadHandle;
		//创建IOCP线程 运行Worker
		if ((ThreadHandle = CreateThread(NULL, 0, Worker, hIocp, 0, &ThreadID)) == NULL){
			printf("CreateThread failed with error: %d\n", GetLastError());
			return false;
		}
		CloseHandle(ThreadHandle);
	}
	WSAData wsd;
	int iResult = WSAStartup(0x0202, &wsd);
	if (iResult != 0){
		printf("WSAStartup failed with error: %d\n", iResult);
		return false;
	}
	return true;

}

bool stop(OverlappedP *Recv){
	if (Recv->socket != INVALID_SOCKET){
		closesocket(Recv->socket);
		Recv->socket = INVALID_SOCKET;
		free(Recv);
		return true;
	}
	return false;
}

OverlappedP* start(int clientport, int callback, int Extra){

	SOCKET sid = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	CreateIoCompletionPort((HANDLE)sid, hIocp, sid, 0);
	//SOCKET sid = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (sid == INVALID_SOCKET){
		printf("socket failed with error %ld\n", WSAGetLastError());
		WSACleanup();
		return NULL;
	}
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons((u_short)clientport);//反转byte排列
	addr.sin_addr.S_un.S_addr = 0;

	int iResult = bind(sid, (LPSOCKADDR)&addr, sizeof(addr));
	if (iResult == SOCKET_ERROR){
		printf("bind failed with eror: %d\n", WSAGetLastError());
		closesocket(sid);
		WSACleanup();
		return NULL;
	}
	OverlappedP* ptr = (OverlappedP *)malloc(sizeof(OverlappedP));
	ptr->wsabuf.len = BUFFER_SIZE;
	ptr->wsabuf.buf = (char *)malloc(BUFFER_SIZE * sizeof(char));
	ptr->OperationType = UDP_RECV_DATA;
	ptr->socket = sid;
	ptr->Callback = callback;
	ptr->Extra = Extra;

	DWORD iFlag = 0;
	int len = sizeof(sockaddr);
	//	LPWSAOVERLAPPED p2 = (LPWSAOVERLAPPED)malloc(sizeof(OVERLAPPED));
	memset(&ptr->overlapped, 0, sizeof(OVERLAPPED));
	iResult = WSARecvFrom(sid, &ptr->wsabuf, 1, &ptr->sendbuflen, &iFlag, (sockaddr *)&ptr->addr, &len, &ptr->overlapped, 0);
	//返回addr信息
	if (iResult == SOCKET_ERROR){
		iResult = WSAGetLastError();
		//#define ERROR_INVALID_HANDLE 6L //The handle is invalid.
		if (iResult != WSA_IO_PENDING){
			return NULL;
		}
	}
	return ptr;
}

bool sendpack(OverlappedP* recv, char* ip, short port, char* buffer, int buflen){
	if (recv->socket == INVALID_SOCKET || recv->socket == SOCKET_ERROR){
		return false;
	}
	u_short uport = htons(port);
	OverlappedP* ptr = (OverlappedP*)malloc(sizeof(OverlappedP));
	ptr->wsabuf.len = buflen;
	ptr->sendbuflen = 0;//已发送长度为0
	ptr->wsabuf.buf = (char*)malloc(buflen * sizeof(char));
	ptr->buf = ptr->wsabuf.buf;
	memcpy(ptr->wsabuf.buf, buffer, buflen);
	ptr->addr.sin_addr.S_un.S_addr = inet_addr(ip);
	ptr->addr.sin_port = uport;
	ptr->addr.sin_family = AF_INET;
	ptr->socket = recv->socket;
	ptr->OperationType = UDP_SEND_DATA;
	ptr->Extra = recv->Extra;
	ptr->Callback = recv->Callback;
	DWORD iFlag = 0;
	memset(&ptr->overlapped, 0, sizeof(OVERLAPPED));
	int ret = WSASendTo(recv->socket, &ptr->wsabuf, 1, NULL, 0, (sockaddr *)&ptr->addr, sizeof(sockaddr_in), (LPOVERLAPPED)ptr, 0);
	if (ret == SOCKET_ERROR){
		ret = WSAGetLastError();
		if (ret != WSA_IO_PENDING){
			//成功启动一个重叠操作，过后将有完成指示。
			return false;
		}
	}
	return true;
}
