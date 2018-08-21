#include "stdafx.h"
#include "TCPClient.h"


HANDLE TCPWorkerhIocp;
bool TCPWorkerisRun;

/*
* 初始化socket环境
*/
bool TCPInitWinsock(){
	SYSTEM_INFO system;
	DWORD ThreadID;
	//创建IOCP
	if ((TCPWorkerhIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL){
		printf("CreateIoCompletionPort failed!\n");
		return false;
	}
	//获取系统信息
	GetSystemInfo(&system);
	//计算最佳线程数量
	TCPWorkerisRun = true;
	int ThreadCount = system.dwNumberOfProcessors * 2 + 2;
	for (int i = 0; i < ThreadCount; i++){
		HANDLE ThreadHandle;
		//创建IOCP线程 运行Worker
		if ((ThreadHandle = CreateThread(NULL, 0, TCPWorker, TCPWorkerhIocp, 0, &ThreadID)) == NULL){
			printf("CreateThread failed with error: %d\n", GetLastError());
			return false;
		}
		//关闭线程句柄
		CloseHandle(ThreadHandle);
	}
	WSAData wsd;
	//初始化winsock环境
	int iResult = WSAStartup(0x0202, &wsd);
	if (iResult != 0){
		printf("WSAStartup failed with error: %d\n", iResult);
		return false;
	}
	
	return true;

}

/*
* 工作线程
*/
DWORD WINAPI TCPWorker(HANDLE iocp){
	DWORD Transferred;
	SOCKET lpCompletekey;//PULONG_PTR
	TCPOverlappedP* lpOverlapped;//LPOVERLAPPED
	int ret = 0;

	while (TCPWorkerisRun){
		if (GetQueuedCompletionStatus(iocp, &Transferred, (PDWORD)&lpCompletekey, (LPOVERLAPPED *)&lpOverlapped, 500)){
			if (lpOverlapped == NULL){
				continue;
			}
			//OverlappedP
			switch (lpOverlapped->OperationType){
			case TCP_RECV_DATA:
				fnRecv(lpOverlapped, Transferred);
				break;
			case TCP_SEND_DATA:
				fnSend(lpOverlapped, Transferred);
				break;
			default:
				break;
			}
		}
		else{
			int dwCode = WSAGetLastError();
			if (dwCode != 258){//TIME_OUT
				printf("WSAGetLastError ret code: %d\n", dwCode);
				TCPCallback funCall = (TCPCallback)lpOverlapped->Callback;
				switch (dwCode){
				case ERROR_NETNAME_DELETED:
					//非正常断开: 服务端/客户端 未调用shutdown就直接调用closesocket(未完成四次挥手).
					//lpOverlapped->errInfo = "异常断开";//TCP连接异常断开, 未完成四次挥手。
					if (lpOverlapped->Callback != 0){
						////事件类型  数据地址 数据长度 服务地址 服务端口 附加数据 错误代码
						
						funCall(TCP_EXCEPTION_DISCONNECT, 0, 0, GetServerAddr(lpOverlapped), GetServerPort(lpOverlapped), lpOverlapped->Extra, TCP_EXCEPTION_DISCONNECT);
						free(lpOverlapped);
					}
					printf("非正常断开: 服务端/客户端 未调用shutdown就直接调用closesocket(未完成四次挥手).\n");
					break;
				case WSA_OPERATION_ABORTED://客户端正常断开：调用shutdownhour调用closesocket就会重现这个代码
					//lpOverlapped->errInfo = "客户断开";
					//TCP客户端主动关闭连接，并且已经完成四次挥手。
					printf("正常断开: TCP客户端主动关闭连接，并且已经完成四次挥手.\n");
					if (lpOverlapped->Callback != 0){
						funCall(TCP_CLIENT_DISCONNECT, 0, 0, GetServerAddr(lpOverlapped), GetServerPort(lpOverlapped), lpOverlapped->Extra, TCP_CLIENT_DISCONNECT);
						//funCall(TCP_CLIENT_DISCONNECT, 0, 0, lpOverlapped);
						free(lpOverlapped);
					}

					//因为套接字的关闭，一个重叠操作被取消，或者是执行了WSAIoctl()函数的SIO_FLUSH命令，错误值依赖于操作系统
					//printf("ret false code: %d\n", c);
					break;
				default:
					break;




				}
			}

		}
	}
	return 0;
}

/*
* TCP_RECV_DATA
*/
void fnRecv(TCPOverlappedP* over, DWORD Transferred){
	unsigned long iFlag = 0;
	int ret = 0;
	TCPCallback funCall = (TCPCallback)over->Callback;
	if (Transferred == 0){
		//1.GetQueuedCompletionStatus返回值为TRUE，则表示客户端是主动断开，TCP / IP协议中断开时的4次握手完成了。
		//2.GetQueuedCompletionStatus返回值为FALSE，则表示客户端是意外断开，4次握手只完成了一部分。SOCKET错误号为：64
		//所以lpNumberOfBytes = 0, 服务端都要处理客户断开的操作。
		//TCP连接正常断开, 已完成四次挥手。
		//over->errInfo = "服务断开";
		if (over->Callback != 0){
			funCall(TCP_SERVER_DISCONNECT, 0, 0, 0, 0, over->Extra, TCP_SERVER_DISCONNECT);
			//funCall(TCP_SERVER_DISCONNECT, 0, 0, over);
			//over->errInfo = 0;
		}
		free(over);
		//printf("TCP连接正常断开, 已完成四次挥手。\n");
		return;
	}

	if (over->Callback != 0){
		funCall(TCP_RECV_DATA, (int)over->wsabuf.buf, Transferred, GetServerAddr(over), GetServerPort(over), over->Extra, 0);
		//funCall(TCP_RECV_DATA, (int)over->wsabuf.buf, Transferred, over);
		//TCP_SERVER_DISCONNECT
	}

	/*
	char* hex = (char *)malloc(Transferred * 2 * sizeof(char)+1);
	Byte2Hex(over->wsabuf.buf, hex, Transferred);
	hex[Transferred * 2] = 0;
	printf("recv=[%s]\n", hex);
	free(hex);
	*/

	ret = WSARecv(over->socket, &over->wsabuf, 1, &over->sendbuflen, &iFlag, &over->overlapped, 0);
	if (ret == SOCKET_ERROR){
		ret = WSAGetLastError();
		//#define ERROR_INVALID_HANDLE 6L //The handle is invalid.
		if (ret != WSA_IO_PENDING){
			//over->errInfo = "WSARecv failed.";
			if (over->Callback != 0){
				//char err[256];
				//sprintf(err, "WSARecv failed with Code: %d.", ret);
				//over->errInfo = err;
				funCall(TCP_ERROR, 0, 0, 0, 0, over->Extra, ret);
				//funCall(TCP_ERROR, 0, 0, over);
				//over->errInfo = NULL;
				//printf("%s\n", err);
				free(over);
			}
			//printf("WSARecv failed!");
		}
	}

}

/*
* 发送成功
*/
void fnSend(TCPOverlappedP* overlapped, DWORD Transferred){

	overlapped->sendbuflen += Transferred;//这是真正发送成功的标志

	/*1:Transferred=1024  overlapped->sendbuflen=1024 overlapped->wsabuf.len=4096-1024     overlapped->wsabuf.buf=overlapped->wsabuf.buf-1024
	2:Transferred=1024  overlapped->sendbuflen=2048 overlapped->wsabuf.len=4096-2048     overlapped->wsabuf.buf=overlapped->wsabuf.buf-2048
	3:Transferred=1024  overlapped->sendbuflen=3072 overlapped->wsabuf.len=4096-3072     overlapped->wsabuf.buf=overlapped->wsabuf.buf-3072
	*/
	


	if (overlapped->sendbuflen < overlapped->wsabuflen){
		overlapped->wsabuf.len -= Transferred;
		overlapped->wsabuf.buf += Transferred;
		int iFlag = 0;
		WSASend(overlapped->socket, &overlapped->wsabuf, 1, NULL, iFlag, (LPWSAOVERLAPPED)&overlapped, 0);
	}
	else{
		

		if (overlapped->Callback != 0){
			TCPCallback funCall = (TCPCallback)overlapped->Callback;
			funCall(TCP_SEND_DATA, (int)overlapped->buf, overlapped->wsabuflen, GetServerAddr(overlapped), GetServerPort(overlapped), overlapped->Extra, 0);
			//funCall(overlapped->OperationType, (int)overlapped->wsabuf.buf, overlapped->wsabuf.len, overlapped);
		}
		free(overlapped->buf);
		free(overlapped);
	}
	printf("send suc\n");


	
}

/*
* 字节数组转十六进制文本
*/
void Byte2Hex(const char *sSrc, char *sDest, int nSrcLen)
{
	int  i;
	char szTmp[3];

	for (i = 0; i < nSrcLen; i++)
	{
		sprintf(szTmp, "%02X", (unsigned char)sSrc[i]);
		memcpy(&sDest[i * 2], szTmp, 2);
	}
	return;
}


/*
* 链接服务器 返回一个结构指针
*/
TCPOverlappedP* Connect(char* serverIp, int serverPort, int callback, int Extra){
	struct sockaddr_in saDest;
	memset(&saDest, 0, sizeof(saDest));

	saDest.sin_addr.S_un.S_addr = inet_addr(serverIp);
	saDest.sin_port = htons((u_short)serverPort);
	saDest.sin_family = AF_INET;

	SOCKET sid = INVALID_SOCKET;
	//创建socket句柄
	sid = socket(AF_INET, SOCK_STREAM, 0);
	if (sid == INVALID_SOCKET){
		printf("Error at socket(): %ld\n", WSAGetLastError());
		return NULL;
	}

	//链接服务器
	int ret = connect(sid, (sockaddr*)&saDest, sizeof(sockaddr));
	if (ret == SOCKET_ERROR){
		printf("connnet failed %d\n", WSAGetLastError());
		return NULL;
	}

	//socket和iocp绑定
	CreateIoCompletionPort((HANDLE)sid, TCPWorkerhIocp, sid, 0);

	TCPOverlappedP* ptr = (TCPOverlappedP *)malloc(sizeof(TCPOverlappedP));
	ptr->wsabuf.len = TCP_BUFFER_SIZE;
	ptr->wsabuf.buf = (char *)malloc(TCP_BUFFER_SIZE * sizeof(char));
	ptr->OperationType = TCP_RECV_DATA;
	ptr->socket = sid;
	ptr->Callback = callback;
	ptr->Extra = Extra;
	struct sockaddr addr;
	int memlen = sizeof(sockaddr);
	//获取客户端口
	getsockname(sid, &addr, &memlen);
	sockaddr_in* addrin = (sockaddr_in *)&addr;
	ptr->wPort = addrin->sin_port;



	DWORD iFlag = 0;
	int len = sizeof(sockaddr);
	//初始结构 以防提示Event句柄无效
	memset(&ptr->overlapped, 0, sizeof(OVERLAPPED));
	//UDP -> ret = WSARecvFrom(sid, &ptr->wsabuf, 1, &ptr->sendbuflen, &iFlag, (sockaddr *)&ptr->addr, &len, &ptr->overlapped, 0);

	//发送WSARecv等待接收数据
	ret = WSARecv(sid, &ptr->wsabuf, 1, &ptr->sendbuflen, &iFlag, &ptr->overlapped, 0);
	if (ret == SOCKET_ERROR){
		ret = WSAGetLastError();
		//#define ERROR_INVALID_HANDLE 6L //The handle is invalid.
		if (ret != WSA_IO_PENDING){
			return NULL;
		}
	}
	return ptr;
}


/*
* TCP_SEND_DATA
* 返回值： -1（SOCKET无效） -2 发送失败 可用WSAGetLastError()获取错误代码
*/
int Send(TCPOverlappedP* recv, char* buf, int len){
	if (recv->socket == INVALID_SOCKET || recv->socket == SOCKET_ERROR){
		return -1;
	}
	TCPOverlappedP* ptr = (TCPOverlappedP*)malloc(sizeof(TCPOverlappedP));
	printf("send ptr: %p\n", ptr);
	ptr->wsabuf.len = len;
	ptr->sendbuflen = 0;//已发送长度为0
	ptr->wsabuflen = len;
	ptr->wsabuf.buf = (char*)malloc(len * sizeof(char));
	ptr->buf = ptr->wsabuf.buf;
	memcpy(ptr->wsabuf.buf, buf, len);
	ptr->addr = recv->addr;

	ptr->socket = recv->socket;
	ptr->OperationType = TCP_SEND_DATA;
	ptr->Extra = recv->Extra;
	ptr->Callback = recv->Callback;
	memset(&ptr->overlapped, 0, sizeof(OVERLAPPED));
	DWORD iFlag = 0;
	int ret = WSASend(ptr->socket, &ptr->wsabuf, 1, NULL, iFlag, (LPWSAOVERLAPPED)&ptr->overlapped, 0);
	if (ret == SOCKET_ERROR){
		ret = WSAGetLastError();
		if (ret != WSA_IO_PENDING){
			TCPCallback funCall = (TCPCallback)recv->Callback;
			//成功启动一个重叠操作，过后将有完成指示。
			char err[256];
			sprintf(err, "WSASend failed with Code: %d.", ret);
			//funCall(TCP_ERROR, 0, 0, recv);
			funCall(TCP_ERROR, 0, 0, GetServerAddr(recv), GetServerPort(recv), recv->Extra, ret);
			printf("send packet error: %d\n", ret);
			return -2;
		}
	}
	//printf("sended len: %d\n", getLen);
	return 0;
}

/*
* 关闭链接
*/
int Close(TCPOverlappedP* recv){
	if (recv->socket == INVALID_SOCKET || recv->socket == SOCKET_ERROR){
		return -1;
	}
	int ret = 0;
	ret = shutdown(recv->socket, SD_SEND);
	if (ret == SOCKET_ERROR){
		return -2;
	}
	ret = closesocket(recv->socket);
	if (ret == SOCKET_ERROR){
		return -3;
	}
	recv->socket = SOCKET_ERROR;
	return ret;
}

void Destory(){
	TCPWorkerisRun = false;
	if (TCPWorkerhIocp > 0){
		PostQueuedCompletionStatus(TCPWorkerhIocp, 0, 0, 0);
		CloseHandle(TCPWorkerhIocp);
		WSACleanup();
	}

}


/*
* 获取本地SOCKET句柄
*/
SOCKET GetSocket(TCPOverlappedP* ptr){
	return ptr->socket;
}

/*
* 获取客户端口
*/
u_short GetClientPort(TCPOverlappedP* ptr){
	return ptr->wPort;
}

/*
* 获取远程端口
*/
u_short GetServerPort(TCPOverlappedP* ptr){
	return ptr->addr.sin_port;
}

/*
* 获取远程IP地址
*/
int GetServerAddr(TCPOverlappedP* ptr){
	return ptr->addr.sin_addr.S_un.S_addr;
}

/*
* 获取附加数据
*/

int GetClientExtra(TCPOverlappedP* ptr){
	return ptr->Extra;
}


