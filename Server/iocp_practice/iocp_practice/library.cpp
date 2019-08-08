#pragma once
#include "iocp.h"


OrderQueue<SentInfo> g_OrderQueue;
WSAEVENT OrderQueueEvent;
RWLock UserMapLock;
SOCKET ListenSocket;
HANDLE CompletionPort;
std::map<SOCKET, User*> UserMap;
Log logger;

using namespace std;

void AddSocket(SOCKET Socket, SOCKADDR_IN Address) {
	UserMapLock.WriteLock();
	printf("socket : %d connected\n", Socket);
	UserMap.insert(std::make_pair(Socket, new User(Socket)));
	UserMap.find(Socket)->second->IP = Address.sin_addr;
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
	printf("delete idx : %d\n", idx);
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
			if (it->second->IsEnter == 1) {
				fromuser = it->second;
				char source[50];
				int sum = 0;
				int namelen = strlen(fromuser->Name);

				DataAddCopy(source, &fromuser->CharacterClass, sizeof(char), sum);
				DataAddCopy(source, &fromuser->PosY, sizeof(float), sum);
				DataAddCopy(source, &fromuser->PosZ, sizeof(float), sum);
				DataAddCopy(source, &fromuser->Damage, sizeof(float), sum);
				DataAddCopy(source, fromuser->Name, namelen, sum);


				shared_ptr<Packet> ko = make_shared<Packet>(PACKET_TYPE::PLAYERSPAWN, sum + FRONTLEN, it->first, source, BROADCAST_MODE::ONLYME);
				SentInfo temp(ko);
				PushAndSend(temp);
			}
		}
	}
	UserMapLock.ReadUnLock();
}

template <typename T>
void DataAddCopy(char * source, T* get, int size, int& sum) {
	memcpy(source+sum, get, size);
	sum += size;
}

template <typename T>
void DataAddGet(T* source, char* get, int size, int& sum) {
	memcpy(source, get+sum, size);
	sum += size;
}

void SpawnProcess(User& myuser, shared_ptr<Packet>& temppacket, int& len) {
	int sum = 0;
	char * source = temppacket.get()->Body + FRONTLEN;
	myuser.IsEnter = 1;

	DataAddGet(&myuser.CharacterClass, source, sizeof(char), sum);
	DataAddGet(&myuser.PosY, source, sizeof(float), sum);
	DataAddGet(&myuser.PosZ, source, sizeof(float), sum);
	DataAddGet(&myuser.Damage, source, sizeof(float), sum);
	myuser.Name[len - sum - FRONTLEN] = '\0';
	DataAddGet(myuser.Name, source, len-sum-FRONTLEN, sum);

	printf("PosY : %f, PosZ : %f\n", myuser.PosY, myuser.PosZ);
	printf("username : %s\n", myuser.Name);

}


shared_ptr<Packet> NameCheckProcess(User& myuser, char * Name, int NameLen) {
	char ReqName[100]{ 0 };
	char buf[1];
	bool isthere = false;
	bool flag = true;

	
	if (NameLen > 20) {
		printf("too long packet name");
		flag = false;
		buf[0] = (char)ERRORCODE::TOOLONG;
	}
	else {
		memcpy(ReqName, Name, NameLen);
		for (int i = 0; i < strlen(ReqName); ++i)
		{
			if ('a' > ReqName[i] || ReqName[i] > 'z') {
				buf[0] = (char)ERRORCODE::NOTENGLISH;
				flag = false;
				break;
			}
		}
	}
	
	if (flag == true) {

		UserMapLock.ReadLock();
		for (auto it = UserMap.begin(); it != UserMap.end(); it++) {
			if (it->second->IsEnter == 1) {
				if (strcmp(ReqName, it->second->Name) == 0) {
					isthere = true;
					break;
				}
			}
		}
		UserMapLock.ReadUnLock();


		if (isthere) {
			printf("중복된이름\n");
			buf[0] = (char)ERRORCODE::ALREADYNAME;
		}
		else {
			printf("welcome!\n");
			buf[0] = (char)ERRORCODE::NOTERROR;
		}
	}


	return make_shared<Packet>(PACKET_TYPE::NAMECHECK, FRONTLEN + 1, myuser.ClientSocket.Socket, buf, BROADCAST_MODE::ONLYME);
	//auto temppacket = make_shared<Packet>(PACKET_TYPE::NAMECHECK, FRONTLEN + 1, myuser.ClientSocket.Socket, buf, BROADCAST_MODE::ONLYME);
	//SentInfo temp(temppacket);
	//myuser.PushAndSend(temp);
}

void RecvProcess(char * source, int retValue, User& myuser) {
	char * receivedBuffer = myuser.ClientSocket.ReceiveBuffer;
	int& receivedSize = myuser.ClientSocket.ReceivedBufferSize;

	memcpy(receivedBuffer + receivedSize, source, retValue);
	receivedSize += retValue;

	
	int len = (int)*(wchar_t*)receivedBuffer;
	logger.write("RecvProcess() before make packet, id:%d len:%d recvSize:%d retvalue:%d", myuser.ClientSocket.Socket, len, receivedSize, retValue);

	int sumlen = 0;

	while (receivedSize >= FRONTLEN && receivedSize >= len) {
		PACKET_TYPE Type = (PACKET_TYPE)*(receivedBuffer + USERLEN + LENLEN + sumlen);
		char * Body = receivedBuffer + FRONTLEN + sumlen;
		shared_ptr<Packet> temppacket;
		// = make_shared<Packet>((PACKET_TYPE)*(receivedBuffer + USERLEN + LENLEN + sumlen), len, myuser.ClientSocket.Socket, receivedBuffer + FRONTLEN + sumlen, BROADCAST_MODE::ALL);

		receivedSize -= len;
		sumlen += len;

		logger.write("RecvProcess() after make packet, id:%d len:%d recvSize:%d sumlen:%d", myuser.ClientSocket.Socket, len, receivedSize, sumlen);
		switch (Type) {
		case PACKET_TYPE::PLAYERSPAWN:
			temppacket = make_shared<Packet>(Type, len, myuser.ClientSocket.Socket, Body, BROADCAST_MODE::ALL);
			SpawnProcess(myuser, temppacket, len);
			break;
		case PACKET_TYPE::UPDATELOCATION:
			temppacket = make_shared<Packet>(Type, len, myuser.ClientSocket.Socket, Body, BROADCAST_MODE::EXCEPTME);
			myuser.PosY = *(float*)(Body);
			myuser.PosZ = *(float*)(Body + 4);
			myuser.Dir = *(Body + 8);
			logger.write("UpdataeLocation id:%d PosY:%f PosZ:%f Dir:%d", myuser.ClientSocket.Socket, myuser.PosY, myuser.PosZ, myuser.Dir);
			break;
		case PACKET_TYPE::UPDATEDMG:
			temppacket = make_shared<Packet>(Type, len, myuser.ClientSocket.Socket, Body, BROADCAST_MODE::EXCEPTME);
			myuser.Damage = *(float*)Body;
			break;
		case PACKET_TYPE::NAMECHECK:
			temppacket = NameCheckProcess(myuser, Body, len - FRONTLEN);
			break;
		case PACKET_TYPE::UPDATESTATE:
			temppacket = make_shared<Packet>(Type, len, myuser.ClientSocket.Socket, Body, BROADCAST_MODE::EXCEPTME);
			if (*Body == (char)ECharacterAction::EA_Die) {
				myuser.IsEnter = 0;
				memset(myuser.Name, 0, sizeof(myuser.Name));
			}
			break;
		}

		SentInfo temp(temppacket);
		g_OrderQueue.Push(temp);
		SetEvent(OrderQueueEvent);

		len = (int)*(wchar_t*)(receivedBuffer + sumlen);
		logger.write("RecvProcess() after(2) make packet, id:%d len:%d recvSize:%d sumlen:%d", myuser.ClientSocket.Socket, len, receivedSize, sumlen);
	}

	for (int k = 0; k < receivedSize; k++)
		receivedBuffer[k + sumlen] = receivedBuffer[k];


}
void Recver::Work(LPPER_HANDLE_DATA PerHandleData, DWORD bytes) {
	SOCKET Socket = PerHandleData->Socket;
	DWORD Flags;
	WSABUF wbuf;

	//logger.write("Recver Work() %d %d", Socket, bytes);
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

	//logger.write("SendFront() sended:%d type:%d", target.Sended, target.Sp.get()->Body[USERLEN+LENLEN]);
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

	logger.write("Send Work() userid:%d bytes:%d type:%d", Socket, bytes, sendinfo.Sp.get()->Type);

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


