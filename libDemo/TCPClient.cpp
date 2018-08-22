#include "stdafx.h"
#include "TCPClient.h"


HANDLE TCPWorkerhIocp;
int TCPThreadCount;
/*
* ��ʼ��socket����
*/
bool TCPInitWinsock(){
	SYSTEM_INFO system;
	DWORD ThreadID;
	//����IOCP
	if ((TCPWorkerhIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL){
		printf("CreateIoCompletionPort failed!\n");
		return false;
	}
	//��ȡϵͳ��Ϣ
	GetSystemInfo(&system);
	//��������߳�����
	TCPThreadCount = system.dwNumberOfProcessors * 2 + 2;
	for (int i = 0; i < TCPThreadCount; i++){
		HANDLE ThreadHandle;
		//����IOCP�߳� ����Worker
		if ((ThreadHandle = CreateThread(NULL, 0, TCPWorker, TCPWorkerhIocp, 0, &ThreadID)) == NULL){
			printf("CreateThread failed with error: %d\n", GetLastError());
			return false;
		}
		//�ر��߳̾��
		CloseHandle(ThreadHandle);
	}
	WSAData wsd;
	//��ʼ��winsock����
	int iResult = WSAStartup(0x0202, &wsd);
	if (iResult != 0){
		printf("WSAStartup failed with error: %d\n", iResult);
		return false;
	}
	
	return true;

}

/*
* �����߳�
*/
DWORD WINAPI TCPWorker(HANDLE iocp){
	DWORD Transferred;
	SOCKET lpCompletekey;//PULONG_PTR
	TCPOverlappedP* lpOverlapped;//LPOVERLAPPED
	int ret = 0;
	BOOL br = false;
	while (true){
		br = GetQueuedCompletionStatus(iocp, &Transferred, (PDWORD)&lpCompletekey, (LPOVERLAPPED *)&lpOverlapped, 0xffffffff);
		if (lpCompletekey == TCP_DESTORY){
			break;
		}

		if(br == TRUE){
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
		}else{
			int dwCode = WSAGetLastError();
			if (dwCode != 258){//TIME_OUT

				if (dwCode == WSA_INVALID_HANDLE){
					break;
				}

				printf("WSAGetLastError ret code: %d\n", dwCode);
				TCPCallback funCall = (TCPCallback)lpOverlapped->Callback;
				switch (dwCode){
				case ERROR_NETNAME_DELETED:
					//�������Ͽ�: �����/�ͻ��� δ����shutdown��ֱ�ӵ���closesocket(δ����Ĵλ���).
					//lpOverlapped->errInfo = "�쳣�Ͽ�";//TCP�����쳣�Ͽ�, δ����Ĵλ��֡�
					if (lpOverlapped->Callback != 0){
						////�¼�����  ���ݵ�ַ ���ݳ��� �����ַ ����˿� �������� �������
						
						funCall(TCP_EXCEPTION_DISCONNECT, 0, 0, GetServerAddr(lpOverlapped), GetServerPort(lpOverlapped), lpOverlapped->Extra, TCP_EXCEPTION_DISCONNECT);
						free(lpOverlapped);
					}
					printf("�������Ͽ�: �����/�ͻ��� δ����shutdown��ֱ�ӵ���closesocket(δ����Ĵλ���).\n");
					break;
				case WSA_OPERATION_ABORTED://�ͻ��������Ͽ�������shutdownhour����closesocket�ͻ������������
					//lpOverlapped->errInfo = "�ͻ��Ͽ�";
					//TCP�ͻ��������ر����ӣ������Ѿ�����Ĵλ��֡�
					printf("�����Ͽ�: TCP�ͻ��������ر����ӣ������Ѿ�����Ĵλ���.\n");
					if (lpOverlapped->Callback != 0){
						funCall(TCP_CLIENT_DISCONNECT, 0, 0, GetServerAddr(lpOverlapped), GetServerPort(lpOverlapped), lpOverlapped->Extra, TCP_CLIENT_DISCONNECT);
						//funCall(TCP_CLIENT_DISCONNECT, 0, 0, lpOverlapped);
						free(lpOverlapped);
					}

					//��Ϊ�׽��ֵĹرգ�һ���ص�������ȡ����������ִ����WSAIoctl()������SIO_FLUSH�������ֵ�����ڲ���ϵͳ
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
		//1.GetQueuedCompletionStatus����ֵΪTRUE�����ʾ�ͻ����������Ͽ���TCP / IPЭ���жϿ�ʱ��4����������ˡ�
		//2.GetQueuedCompletionStatus����ֵΪFALSE�����ʾ�ͻ���������Ͽ���4������ֻ�����һ���֡�SOCKET�����Ϊ��64
		//����lpNumberOfBytes = 0, ����˶�Ҫ����ͻ��Ͽ��Ĳ�����
		//TCP���������Ͽ�, ������Ĵλ��֡�
		//over->errInfo = "����Ͽ�";
		if (over->Callback != 0){
			funCall(TCP_SERVER_DISCONNECT, 0, 0, 0, 0, over->Extra, TCP_SERVER_DISCONNECT);
			//funCall(TCP_SERVER_DISCONNECT, 0, 0, over);
			//over->errInfo = 0;
		}
		free(over);
		//printf("TCP���������Ͽ�, ������Ĵλ��֡�\n");
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
* ���ͳɹ�
*/
void fnSend(TCPOverlappedP* overlapped, DWORD Transferred){

	TCPCallback funCall = (TCPCallback)overlapped->Callback;

	if (Transferred == 0){
		//1.GetQueuedCompletionStatus����ֵΪTRUE�����ʾ�ͻ����������Ͽ���TCP / IPЭ���жϿ�ʱ��4����������ˡ�
		//2.GetQueuedCompletionStatus����ֵΪFALSE�����ʾ�ͻ���������Ͽ���4������ֻ�����һ���֡�SOCKET�����Ϊ��64
		//����lpNumberOfBytes = 0, ����˶�Ҫ����ͻ��Ͽ��Ĳ�����
		//TCP���������Ͽ�, ������Ĵλ��֡�
		//over->errInfo = "����Ͽ�";
		if (overlapped->Callback != 0){
			funCall(TCP_SERVER_DISCONNECT, 0, 0, 0, 0, overlapped->Extra, TCP_SERVER_DISCONNECT);
			//funCall(TCP_SERVER_DISCONNECT, 0, 0, over);
			//over->errInfo = 0;
		}
		free(overlapped);
		//printf("TCP���������Ͽ�, ������Ĵλ��֡�\n");
		return;
	}

	overlapped->sendbuflen += Transferred;//�����������ͳɹ��ı�־

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
			
			funCall(TCP_SEND_DATA, (int)overlapped->buf, overlapped->wsabuflen, GetServerAddr(overlapped), GetServerPort(overlapped), overlapped->Extra, 0);
			//funCall(overlapped->OperationType, (int)overlapped->wsabuf.buf, overlapped->wsabuf.len, overlapped);
		}
		free(overlapped->buf);
		free(overlapped);
	}
	printf("send suc\n");


	
}

/*
* �ֽ�����תʮ�������ı�
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
* ���ӷ����� ����һ���ṹָ��
*/
TCPOverlappedP* Connect(char* serverIp, int serverPort, int callback, int Extra){
	struct sockaddr_in saDest;
	memset(&saDest, 0, sizeof(saDest));

	saDest.sin_addr.S_un.S_addr = inet_addr(serverIp);
	saDest.sin_port = htons((u_short)serverPort);
	saDest.sin_family = AF_INET;

	SOCKET sid = INVALID_SOCKET;
	//����socket���
	sid = socket(AF_INET, SOCK_STREAM, 0);
	if (sid == INVALID_SOCKET){
		printf("Error at socket(): %ld\n", WSAGetLastError());
		return NULL;
	}

	//���ӷ�����
	int ret = connect(sid, (sockaddr*)&saDest, sizeof(sockaddr));
	if (ret == SOCKET_ERROR){
		printf("connnet failed %d\n", WSAGetLastError());
		return NULL;
	}

	//socket��iocp��
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
	//��ȡ�ͻ��˿�
	getsockname(sid, &addr, &memlen);
	sockaddr_in* addrin = (sockaddr_in *)&addr;
	ptr->wPort = addrin->sin_port;



	DWORD iFlag = 0;
	int len = sizeof(sockaddr);
	//��ʼ�ṹ �Է���ʾEvent�����Ч
	memset(&ptr->overlapped, 0, sizeof(OVERLAPPED));
	//UDP -> ret = WSARecvFrom(sid, &ptr->wsabuf, 1, &ptr->sendbuflen, &iFlag, (sockaddr *)&ptr->addr, &len, &ptr->overlapped, 0);

	//����WSARecv�ȴ���������
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
* ����ֵ�� -1��SOCKET��Ч�� -2 ����ʧ�� ����WSAGetLastError()��ȡ�������
*/
int Send(TCPOverlappedP* recv, char* buf, int len){
	if (recv->socket == INVALID_SOCKET || recv->socket == SOCKET_ERROR){
		return -1;
	}
	TCPOverlappedP* ptr = (TCPOverlappedP*)malloc(sizeof(TCPOverlappedP));
	printf("send ptr: %p\n", ptr);
	ptr->wsabuf.len = len;
	ptr->sendbuflen = 0;//�ѷ��ͳ���Ϊ0
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
			//�ɹ�����һ���ص����������������ָʾ��
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
* �ر�����
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
	if (TCPWorkerhIocp > 0){
		for (int i = 0; i < TCPThreadCount; i++){
			PostQueuedCompletionStatus(TCPWorkerhIocp, 0, TCP_DESTORY, 0);
		}
		CloseHandle(TCPWorkerhIocp);
		WSACleanup();
	}

}


/*
* ��ȡ����SOCKET���
*/
SOCKET GetSocket(TCPOverlappedP* ptr){
	return ptr->socket;
}

/*
* ��ȡ�ͻ��˿�
*/
u_short GetClientPort(TCPOverlappedP* ptr){
	return ptr->wPort;
}

/*
* ��ȡԶ�̶˿�
*/
u_short GetServerPort(TCPOverlappedP* ptr){
	return ptr->addr.sin_port;
}

/*
* ��ȡԶ��IP��ַ
*/
int GetServerAddr(TCPOverlappedP* ptr){
	return ptr->addr.sin_addr.S_un.S_addr;
}

/*
* ��ȡ��������
*/

int GetClientExtra(TCPOverlappedP* ptr){
	return ptr->Extra;
}


