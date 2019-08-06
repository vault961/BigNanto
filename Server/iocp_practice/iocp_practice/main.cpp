#pragma once
#include "iocp.h"

#include <thread>
#include <process.h>

using namespace std;


extern OrderQueue<SentInfo> g_OrderQueue;
extern WSAEVENT OrderQueueEvent;
extern RWLock UserMapLock;
extern SOCKET ListenSocket;
extern HANDLE CompletionPort;
extern std::map<SOCKET, User*> UserMap;


void OrderQueueThread() {
	while (1) {
		WaitForSingleObject(OrderQueueEvent, INFINITE);
		while (!g_OrderQueue.empty()) {
			SentInfo temp(g_OrderQueue.Front());
			g_OrderQueue.Pop();
			
			UserMapLock.ReadLock();
			for (auto it = UserMap.begin();it != UserMap.end(); it++)
				(it->second)->PushAndSend(temp);
			UserMapLock.ReadUnLock();
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
			char empty[1]{ 0 };
			auto temppacket = make_shared<Packet>(PACKET_TYPE::LOGOUT, FRONTLEN+1, PerHandleData->Socket, empty);
			SentInfo temp(temppacket);
			// broadcast
			g_OrderQueue.Push(temp);
			SetEvent(OrderQueueEvent);


			//close socket
			printf("close!");
			closesocket(PerHandleData->Socket);
			CompressArrays(PerHandleData->Socket);

			free(PerHandleData);
			delete (Worker*)RecvData;
			continue;
		}
		((Worker*)RecvData)->Work(PerHandleData, BytesTransferred);
		
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
	retValue = ::bind(ListenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (retValue == SOCKET_ERROR)
		return -1;

	// sendqueue event
	OrderQueueEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	//listen
	listen(ListenSocket, MAX_USER);
	
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

		UserMapLock.ReadLock();
		if (UserMap.size() >= WSA_MAXIMUM_WAIT_EVENTS) {
			printf("too many connections");
			closesocket(Accept);
			break;
		}
		UserMapLock.ReadUnLock();

		AddSocket(Accept);

		PerHandleData = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		PerHandleData->Socket = Accept;
		memcpy(&(PerHandleData->ClientAddr), &saRemote, RemoteLen);

		CreateIoCompletionPort((HANDLE)Accept, CompletionPort, (DWORD)PerHandleData, 0);

		UserMapLock.ReadLock();
		(UserMap.find(Accept)->second)->GetOthersInfo();
		UserMapLock.ReadUnLock();
		

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

