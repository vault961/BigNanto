#pragma once
#include "iocp.h"


OrderQueue<SentInfo> g_OrderQueue;
WSAEVENT OrderQueueEvent;
RWLock UserMapLock;
SOCKET ListenSocket;
HANDLE CompletionPort;
std::unordered_map<char, User*> UserMap;
using namespace std;

char AddSocket(int Socket) {
	UserMapLock.WriteLock();
	hash<int> h;
	char idx = h(Socket);
	printf("socket : %c connected\n", idx);
	UserMap.insert(std::make_pair(idx, new User(idx, Socket)));
	UserMapLock.WriteUnLock();
	return idx;
}

User& GetUser(char idx) {
	UserMapLock.ReadLock();
	//User& myuser = UserList[idx];
	User& myuser = *(UserMap.find(idx)->second);
	UserMapLock.ReadUnLock();
	return myuser;
}

void CompressArrays(char idx) {
	printf("delete idx : %c\n", idx);
	UserMapLock.WriteLock();
	delete UserMap.find(idx)->second;
	UserMap.erase(idx);
	UserMapLock.WriteUnLock();
}
void Compress(char *source, int len) {
	for (int k = 0; k < len; k++) 
		source[k + len] = source[k];
}

void PushAndSend() {

}
void User::GetOthersInfo() {
	User* fromuser;
	char idx = Idx;
	UserMapLock.ReadLock();
	for (auto it = UserMap.begin();it != UserMap.end(); it++) {
		if (it->first != idx) {
			fromuser = it->second;
			int namelen = strlen(fromuser->Name);
			if (namelen > 0) {
				char buf[50];
				
				memcpy(buf, &fromuser->CharacterClass, sizeof(char));
				memcpy(buf + sizeof(char), &fromuser->PosY, sizeof(float));
				memcpy(buf + sizeof(char) + sizeof(float), &fromuser->PosZ, sizeof(float));
				memcpy(buf + sizeof(char) + sizeof(float)*2, &fromuser->Damage, sizeof(wchar_t));
				memcpy(buf + sizeof(char) + sizeof(float)*2 + sizeof(wchar_t), fromuser->Name, namelen);

				auto ko = make_shared<Packet>(PACKET_TYPE::PLAYERSPAWN, namelen + FRONTLEN + sizeof(float)*2 + sizeof(wchar_t), it->first, fromuser->Name);
				SentInfo temp(ko);

				//PushAndSend(temp);
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
	UserMapLock.ReadUnLock();
}

void SpawnProcess(User& myuser, shared_ptr<Packet>& temppacket, wchar_t& len) {
	myuser.CharacterClass = *temppacket->Body;
	myuser.PosY = *(float*)(temppacket->Body + 1);
	myuser.PosZ = *(float*)(temppacket->Body + sizeof(float) + 1);
	myuser.Damage = *(wchar_t*)(temppacket->Body + sizeof(float) * 2 + 1);
	memcpy(myuser.Name, temppacket->Body + sizeof(float) * 3 + 1, len - FRONTLEN - 1 - sizeof(float) * 3);
	myuser.Name[len - FRONTLEN] = 0;

	printf("username :%s ", myuser.Name);
}

void EnterProcess(User& myuser, shared_ptr<Packet>& temppacket) {
	temppacket.get()->Body[FRONTLEN] = myuser.Idx;
	SentInfo temp(temppacket);
	myuser.ClientSocket.WaitingQueue.Push(temp);
	Sender* PerIoData = new Sender();
	myuser.SendFront(PerIoData);
}

void RecvProcess(char * source, int retValue, User& myuser) {
	char * receiveBuffer = myuser.ClientSocket.ReceiveBuffer;
	int& receivedSize = myuser.ClientSocket.ReceivedBufferSize;
	
	memcpy(receiveBuffer + receivedSize, source, retValue);
	receivedSize += retValue;

	if (receivedSize < FRONTLEN)   // 헤더는 무조건 있어야됨
		return;

	wchar_t len = *(wchar_t*)(receiveBuffer + TYPELEN + USERLEN);
	printf("%d recieve\n", len);

	if (receivedSize >= len) {
		auto temppacket = make_shared<Packet>((PACKET_TYPE)*receiveBuffer, len, myuser.Idx, receiveBuffer + FRONTLEN);
		Compress(myuser.ClientSocket.ReceiveBuffer, receivedSize - len); // array resize
		myuser.ClientSocket.ReceivedBufferSize -= len;

		switch (temppacket->Type) {
		case PACKET_TYPE::PLAYERSPAWN:
			SpawnProcess(myuser, temppacket, len);
			break;
		case PACKET_TYPE::ENTER:
			// do not broadcast
			EnterProcess(myuser, temppacket);
			return;
			break;
		}
		

		SentInfo temp(temppacket);
		// broadcast
		g_OrderQueue.Push(temp);
		SetEvent(OrderQueueEvent);
	}

}
void Recver::Work(LPPER_HANDLE_DATA PerHandleData, DWORD bytes) {
	SOCKET Socket = PerHandleData->Socket;
	char idx = PerHandleData->idx;
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
	wBuf.buf = target.Sp.get()->Body;
	wBuf.len = target.MaxLen;
	overlapped->sendinfo = target;
	WSASend(ClientSocket.Socket, &wBuf, 1, NULL, 0, overlapped, NULL);
}

void Sender::Work(LPPER_HANDLE_DATA PerHandleData, DWORD bytes) {
	SOCKET Socket = PerHandleData->Socket;
	char idx = PerHandleData->idx;
	User& myuser = GetUser(idx);
	WSABUF wBuf;
	sendinfo.Sended += bytes;

	if (sendinfo.Sended >= sendinfo.MaxLen) {
		myuser.ClientSocket.WaitingQueue.Pop();
		myuser.SendFront(this);
	}
	else
	{
		wBuf.buf = sendinfo.Sp.get()->Body + sendinfo.Sended;
		wBuf.len = sendinfo.MaxLen - sendinfo.Sended;
		WSASend(myuser.ClientSocket.Socket, &wBuf, 1, NULL, 0, this, NULL);
	}
}


