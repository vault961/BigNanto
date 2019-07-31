#pragma once
#include "iocp.h"
#include <memory>

OrderQueue<Packet> g_OrderQueue;
User UserList[MAX_USER];
WSAEVENT OrderQueueEvent;
char bufTable[100][MAX_PACKET_SIZE];
std::vector<int> UserVec;
RWLock UserVecLock;
RWLock UserListLock;
SOCKET ListenSocket;
HANDLE CompletionPort;

int AddSocket(int Socket) {
	int idx;
	UserListLock.WriteLock();
	UserVecLock.WriteLock();
	for (int i = 1; i < MAX_USER; i++) {
		if (UserList[i].Connection == FALSE) {
			UserList[i].Connection = TRUE;
			UserVec.push_back(i);
			idx = i;
			break;
		}
	}
	printf("add socket idx : %d\n", idx);
	User * myuser = &UserList[idx];
	myuser->Idx = idx;
	myuser->ClientSocket.Socket = Socket;
	myuser->Name[0] = NULL;

	UserVecLock.WriteUnLock();
	UserListLock.WriteUnLock();

	return idx;
}

User& GetUser(int idx) {
	UserListLock.ReadLock();
	User& myuser = UserList[idx];
	UserListLock.ReadUnLock();
	return myuser;
}

void CompressArrays(int idx) {
	printf("delete idx : %d\n", idx);
	UserListLock.WriteLock();
	UserVecLock.WriteLock();
	UserList[idx].Connection = FALSE;
	for (int k = 0; k < UserVec.size(); k++) {
		if (UserVec[k] == idx) {
			UserVec.erase(UserVec.begin() + k);
			break;
		}
	}
	UserVecLock.WriteUnLock();
	UserListLock.WriteUnLock();
}

void Compress(char *source, int len) {
	// len 만큼 당기지말고
	// 전체에서 len 뺀 대상에대해 줄여야됨.
	for (int k = 0; k < len; k++)  // buf 당기기
		source[k + len] = source[k];
}


void CreateAccount(Packet *temppacket, char* name, char* buf, int len) {
	temppacket->Type = PACKET_TYPE::MYLOGIN;
	memcpy(name, buf, len);
	name[len] = 0;

	printf("%s 계정 생성\n", name);
}

void User::GetOthersInfo() {
	User* fromuser;
	int idx = Idx;
	UserVecLock.ReadLock();
	UserListLock.ReadLock();
	for (int j = 0; j < UserVec.size(); j++) {
		if (UserVec[j] != idx) {
			fromuser = &UserList[UserVec[j]];
			int namelen = strlen(fromuser->Name);
			if (namelen > 0) {
				Packet temppacket(PACKET_TYPE::OTHERLOGIN, namelen + 6, UserVec[j], fromuser->Name);
				SentInfo temp(temppacket);

				if (ClientSocket.WaitingQueue.empty()) {
					ClientSocket.WaitingQueue.Push(temp);
					Sender* PerIoData = new Sender();
					SendFront(PerIoData);
				}
				else
				{
					ClientSocket.WaitingQueue.Push(temp);
				}
			}
		}
	}
	UserVecLock.ReadUnLock();
	UserListLock.ReadUnLock();
}

void RecvProcess(char * source, int retValue, User& myuser) {

	char * receiveBuffer = myuser.ClientSocket.ReceiveBuffer;
	int& receivedSize = myuser.ClientSocket.ReceivedBufferSize;
	

	memcpy(receiveBuffer + receivedSize, source, retValue);
	receivedSize += retValue;

	unsigned int len = *(unsigned int*)(receiveBuffer + 2);
	printf("%d recieve\n", len);

	if (receivedSize >= len) {
		Packet temppacket((PACKET_TYPE)*receiveBuffer, len, myuser.Idx, receiveBuffer + 6);

		// if user hasn't name, put name
		if (temppacket.Type == PACKET_TYPE::MYLOGIN) {
			CreateAccount(&temppacket, myuser.Name, myuser.ClientSocket.ReceiveBuffer + 6, len - 6);
			printf("username :%s ", myuser.Name);
		}
		Compress(myuser.ClientSocket.ReceiveBuffer, receivedSize-len); // buf 당기기

		myuser.ClientSocket.ReceivedBufferSize -= len;  // buf resize
		//printf("packet : %d\n", len + TIMESTAMPLEN);

		g_OrderQueue.Push(temppacket);
		SetEvent(OrderQueueEvent);
	}

}
void Recver::Work(SOCKET Socket, int idx, DWORD bytes) {
	DWORD Flags;
	WSABUF wbuf;

	User& myuser = GetUser(idx);
	RecvProcess(Buffer, bytes, myuser);

	Flags = 0;
	BufferLen = MAX_BUFFER_SIZE;
	wbuf.buf = Buffer;
	wbuf.len = BufferLen;
	WSARecv(Socket, &wbuf, 1, NULL, &Flags, this, NULL);
}

void User::SendFront(Sender* overlapped) {
	WSABUF wBuf;
	if (ClientSocket.WaitingQueue.empty()) {
		delete overlapped;
		return;
	}
	
	SentInfo &target = ClientSocket.WaitingQueue.Front();
	wBuf.buf = target.BufRef;
	wBuf.len = target.MaxLen;
	overlapped->sendinfo = target;
	WSASend(ClientSocket.Socket, &wBuf, 1, NULL, 0, overlapped, NULL);
}

void Sender::Work(SOCKET Socket, int idx, DWORD bytes) {
	User& myuser = GetUser(idx);
	WSABUF wBuf;
	sendinfo.Sended += bytes;

	if (sendinfo.Sended >= sendinfo.MaxLen) {
		myuser.ClientSocket.WaitingQueue.Pop();
		myuser.SendFront(this);
	}
	else
	{
		wBuf.buf = sendinfo.BufRef + sendinfo.Sended;
		wBuf.len = sendinfo.MaxLen - sendinfo.Sended;
		WSASend(myuser.ClientSocket.Socket, &wBuf, 1, NULL, 0, this, NULL);
	}
}
