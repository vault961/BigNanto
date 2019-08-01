#pragma once
#include "iocp.h"

#include <thread>
#include <process.h>

extern OrderQueue<Packet> g_OrderQueue;
extern User UserList[MAX_USER];
extern WSAEVENT OrderQueueEvent;
extern std::vector<int> UserVec;
extern RWLock UserVecLock;
extern RWLock UserListLock;
extern SOCKET ListenSocket;
extern HANDLE CompletionPort;


void OrderQueueThread() {
	while (1) {
		WaitForSingleObject(OrderQueueEvent, INFINITE);
		while (!g_OrderQueue.empty()) {
			Packet pc(g_OrderQueue.Pop());
			SentInfo temp(pc);
			
			UserVecLock.ReadLock();
			UserListLock.ReadLock();
			for (int j = 0; j < UserVec.size(); j++) {
				User * targetuser = &UserList[UserVec[j]];
				if (pc.Type == PACKET_TYPE::MYLOGIN) {
					if (UserVec[j] != pc.UserID) {
						temp.BufRef[0] = (char)(PACKET_TYPE::OTHERLOGIN);
					}
					else {
						temp.BufRef[0] = (char)(PACKET_TYPE::MYLOGIN);
					}
				}
				if (targetuser->ClientSocket.WaitingQueue.empty()) {
					targetuser->ClientSocket.WaitingQueue.Push(temp);
					Sender* PerIoData = new Sender();
					targetuser->SendFront(PerIoData);
				}
				else
				{
					targetuser->ClientSocket.WaitingQueue.Push(temp);
				}
			}
			UserListLock.ReadUnLock();
			UserVecLock.ReadUnLock();
		}
	}
}

unsigned int __stdcall ServerWorkerThread(LPVOID CompletionPortID) {
	HANDLE CompletionPort = (HANDLE)CompletionPortID;
	DWORD BytesTransferred;
	LPPER_HANDLE_DATA PerHandleData = NULL;
	LPOVERLAPPED RecvData;
	int ret;

	while (1) {
		ret = GetQueuedCompletionStatus(CompletionPort, &BytesTransferred, (LPDWORD)&PerHandleData, (LPOVERLAPPED*)&RecvData, INFINITE);
		if (BytesTransferred == 0) {
			//close socket
			printf("close!");
			closesocket(PerHandleData->Socket);
			CompressArrays(PerHandleData->idx);

			free(PerHandleData);
			delete (Worker*)RecvData;
			continue;
		}

		((Worker*)RecvData)->Work(PerHandleData->Socket, PerHandleData->idx, BytesTransferred);
	}
	return 0;
}

int main(void) {
	int retValue;
	
	SYSTEM_INFO SystemInfo;
	WSANETWORKEVENTS networkEvents;
	//wsa
	WSADATA wsd;
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {
		return -1;
	}

	// create i/o completion port

	CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	GetSystemInfo(&SystemInfo);
	//SystemInfo.dwNumberOfProcessors
	for (int i = 0; i < SystemInfo.dwNumberOfProcessors; i++) {
		_beginthreadex(NULL, 0, ServerWorkerThread, (LPVOID)CompletionPort, 0, NULL);
	}

	//socket
	
	ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	//bind
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(27015);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	retValue = bind(ListenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (retValue == SOCKET_ERROR)
		return -1;

	// sendqueue event
	OrderQueueEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	//listen
	listen(ListenSocket, 20);
	
	std::thread th1(OrderQueueThread);
	Recver * PerIoData;
	LPPER_HANDLE_DATA PerHandleData;
	DWORD flags;
	WSABUF wbuf;
	SOCKADDR_IN saRemote;
	int RemoteLen;
	SOCKET Accept;


	while (1) {

		RemoteLen = sizeof(saRemote);
		Accept = accept(ListenSocket, (SOCKADDR*)&saRemote, &RemoteLen);

		if (UserVec.size() >= WSA_MAXIMUM_WAIT_EVENTS) {
			printf("too many connections");
			closesocket(Accept);
			break;
		}
		printf("socket : %d connected\n", Accept);

		int myidx = AddSocket(Accept);

		PerHandleData = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		PerHandleData->Socket = Accept;
		PerHandleData->idx = myidx;
		memcpy(&(PerHandleData->ClientAddr), &saRemote, RemoteLen);

		CreateIoCompletionPort((HANDLE)Accept, CompletionPort, (DWORD)PerHandleData, 0);

		UserList[myidx].GetOthersInfo();

		PerIoData = new Recver();
		flags = 0;
		PerIoData->BufferLen = MAX_BUFFER_SIZE;
		wbuf.buf = PerIoData->Buffer;
		wbuf.len = PerIoData->BufferLen;
		WSARecv(PerHandleData->Socket, &wbuf, 1, NULL, &flags, PerIoData, NULL);
	}
	th1.join();
	printf("server end");
	CloseHandle(OrderQueueEvent);

	// distroy
	WSACleanup();
	return 0;

}

