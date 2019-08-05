#pragma once
#include "iocp.h"


OrderQueue<SentInfo> g_OrderQueue;
WSAEVENT OrderQueueEvent;
RWLock UserMapLock;
SOCKET ListenSocket;
HANDLE CompletionPort;
std::map<SOCKET, User*> UserMap;
using namespace std;

void AddSocket(SOCKET Socket) {
	UserMapLock.WriteLock();
	printf("socket : %d connected\n", Socket);
	UserMap.insert(std::make_pair(Socket, new User(Socket)));
	UserMapLock.WriteUnLock();
}

User& GetUser(SOCKET idx) {
	UserMapLock.ReadLock();
	//User& myuser = UserList[idx];
	User& myuser = *(UserMap.find(idx)->second);
	UserMapLock.ReadUnLock();
	return myuser;
}

void CompressArrays(SOCKET idx) {
	printf("delete idx : %c\n", idx);
	UserMapLock.WriteLock();
	UserMap.erase(idx);
	UserMapLock.WriteUnLock();
}
void Compress(char *source, int len) {
	for (int k = 0; k < len; k++) 
		source[k + len] = source[k];
}

void User::PushAndSend(SentInfo& temp) {
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

void User::GetOthersInfo() {
	User* fromuser;
	UserMapLock.ReadLock();
	for (auto it = UserMap.begin();it != UserMap.end(); it++) {
		if (it->first != ClientSocket.Socket) {
			fromuser = it->second;
			char source[50];
			int sum = 0;
			int namelen = strlen(fromuser->Name);

			DataAddCopy(source, &fromuser->CharacterClass, sizeof(char), sum);
			DataAddCopy(source, &fromuser->PosY, sizeof(float), sum);
			DataAddCopy(source, &fromuser->PosZ, sizeof(float), sum);
			DataAddCopy(source, &fromuser->Damage, sizeof(wchar_t), sum);
			DataAddCopy(source, fromuser->Name, namelen, sum);


			auto ko = make_shared<Packet>(PACKET_TYPE::PLAYERSPAWN, sum, it->first, source);
			SentInfo temp(ko);
			PushAndSend(temp);
		}
	}
	UserMapLock.ReadUnLock();
}

template <typename T>
void DataAddCopy(char * source, T* get, int size, int& sum) {
	memcpy(source+sum, get, size);
	sum += size;
}

void SpawnProcess(User& myuser, shared_ptr<Packet>& temppacket, wchar_t& len) {
	int sum = 0;
	char * source = temppacket.get()->Body + FRONTLEN;
	DataAddCopy(source, &myuser.CharacterClass, sizeof(char), sum);
	DataAddCopy(source, &myuser.PosY, sizeof(float), sum);
	DataAddCopy(source, &myuser.PosZ, sizeof(float), sum);
	DataAddCopy(source, &myuser.Damage, sizeof(wchar_t), sum);
	myuser.Name[len - sum - FRONTLEN] = '\0';
	DataAddCopy(source, myuser.Name, len-sum-FRONTLEN, sum);

	printf("%f %f\n", myuser.PosY, myuser.PosZ);
		
	printf("username :%s ", myuser.Name);
}

void EnterProcess(User& myuser, shared_ptr<Packet>& temppacket) {
	memcpy(temppacket.get()->Body + LENLEN, &myuser.ClientSocket.Socket, USERLEN);
	SentInfo temp(temppacket);
	myuser.PushAndSend(temp);
}

void RecvProcess(char * source, int retValue, User& myuser) {
	char * receiveBuffer = myuser.ClientSocket.ReceiveBuffer;
	int& receivedSize = myuser.ClientSocket.ReceivedBufferSize;
	
	memcpy(receiveBuffer + receivedSize, source, retValue);
	receivedSize += retValue;

	if (receivedSize < FRONTLEN)   // 헤더는 무조건 있어야됨
		return;

	wchar_t len = *(wchar_t*)receiveBuffer;
	printf("%d recieve\n", len);

	if (receivedSize >= len) {
		auto temppacket = make_shared<Packet>((PACKET_TYPE)*receiveBuffer, len, myuser.ClientSocket.Socket, receiveBuffer + FRONTLEN);
		Compress(myuser.ClientSocket.ReceiveBuffer, receivedSize - len); // array resize
		myuser.ClientSocket.ReceivedBufferSize -= len;

		switch (temppacket.get()->Type) {
		case PACKET_TYPE::PLAYERSPAWN:
			SpawnProcess(myuser, temppacket, len);
			break;
		case PACKET_TYPE::ENTER:
			// do not broadcast
			EnterProcess(myuser, temppacket);
			return;
			break;
		case PACKET_TYPE::UPDATELOCATION:
			printf("%f %f\n", *(float*)(temppacket.get()->Body + 4), *(float*)(temppacket.get()->Body+ 8 ));
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
	DWORD Flags;
	WSABUF wbuf;


	User& myuser = GetUser(Socket);
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
	User& myuser = GetUser(Socket);
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


