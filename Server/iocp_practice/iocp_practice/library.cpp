#pragma once
#include "iocp.h"
#include <memory>

OrderQueue<Packet> g_OrderQueue;
User UserList[MAX_USER];
WSAEVENT OrderQueueEvent;
std::vector<int> UserVec;
RWLock UserVecLock;
RWLock UserListLock;
SOCKET ListenSocket;
HANDLE CompletionPort;

int AddSocket(int Socket) {
	int idx;
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
	UserVecLock.WriteLock();
	UserList[idx].Connection = FALSE;
	for (int k = 0; k < UserVec.size(); k++) {
		if (UserVec[k] == idx) {
			UserVec.erase(UserVec.begin() + k);
			break;
		}
	}
	UserVecLock.WriteUnLock();
}

void Compress(char *source, int len) {
	// len 만큼 당기지말고
	// 전체에서 len 뺀 대상에대해 줄여야됨.
	for (int k = 0; k < len; k++)  // buf 당기기
		source[k + len] = source[k];
}



void User::GetOthersInfo() {
	User* fromuser;
	int idx = Idx;
	UserVecLock.ReadLock();
	for (int j = 0; j < UserVec.size(); j++) {
		if (UserVec[j] != idx) {
			fromuser = &UserList[UserVec[j]];
			int namelen = strlen(fromuser->Name);
			if (namelen > 0) {
				char buf[50];
				
				memcpy(buf, &fromuser->Kind, sizeof(char));
				memcpy(buf + sizeof(char), &fromuser->PosY, sizeof(float));
				memcpy(buf + sizeof(char) + sizeof(float), &fromuser->PosZ, sizeof(float));
				memcpy(buf + sizeof(char) + sizeof(float)*2, &fromuser->Damage, sizeof(wchar_t));
				memcpy(buf + sizeof(char) + sizeof(float)*2 + sizeof(wchar_t), fromuser->Name, namelen);

				Packet temppacket(PACKET_TYPE::LOGIN, namelen + FRONTLEN + sizeof(float)*2 + sizeof(wchar_t), UserVec[j], fromuser->Name);
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
}

void RecvProcess(char * source, int retValue, User& myuser) {

	char * receiveBuffer = myuser.ClientSocket.ReceiveBuffer;
	int& receivedSize = myuser.ClientSocket.ReceivedBufferSize;
	

	memcpy(receiveBuffer + receivedSize, source, retValue);
	receivedSize += retValue;

	if (receivedSize < FRONTLEN)
		return;

	wchar_t len = *(wchar_t*)(receiveBuffer + TYPELEN + USERLEN);
	printf("%d recieve\n", len);

	if (receivedSize >= len) {
		Packet temppacket((PACKET_TYPE)*receiveBuffer, len, myuser.Idx, receiveBuffer + FRONTLEN);
		Compress(myuser.ClientSocket.ReceiveBuffer, receivedSize - len); // buf 당기기
		myuser.ClientSocket.ReceivedBufferSize -= len;  // buf resize

		// if user hasn't name, put name
		if (temppacket.Type == PACKET_TYPE::LOGIN) {
			myuser.Class = *temppacket.Body;
			myuser.PosY = *(float*)(temppacket.Body + 1);
			myuser.PosZ = *(float*)(temppacket.Body + sizeof(float) + 1);
			myuser.Damage = *(wchar_t*)(temppacket.Body + sizeof(float) * 2 + 1);
			memcpy(myuser.Name, temppacket.Body + sizeof(float)*3 + 1, len - FRONTLEN - 1 - sizeof(float)*3);
			myuser.Name[len - FRONTLEN] = 0;

			printf("username :%s ", myuser.Name);
		}
		else if (temppacket.Type == PACKET_TYPE::ENTER) {
			temppacket.Body[0] = myuser.Idx;
			SentInfo temp(temppacket);
			myuser.ClientSocket.WaitingQueue.Push(temp);
			Sender* PerIoData = new Sender();
			myuser.SendFront(PerIoData);

			return;
		}
		// broadcast
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
