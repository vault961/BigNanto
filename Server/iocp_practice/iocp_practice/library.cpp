#pragma once
#include "iocp.h"
#include <iostream>

OrderQueue<SentInfo> g_OrderQueue;
WSAEVENT OrderQueueEvent;
RWLock UserMapLock;
SOCKET ListenSocket;
HANDLE CompletionPort;
std::map<SOCKET, User*> UserMap;
Log logger;
std::map<ULONG, int> IPMap;
CRITICAL_SECTION g_OrderQueueLock;
//CRITICAL_SECTION UserMapLock;

using namespace std;

void AddSocket(SOCKET Socket, SOCKADDR_IN Address) {
	UserMapLock.WriteLock();

	printf("socket : %d connected\n", Socket);
	UserMap.insert(std::make_pair(Socket, new User(Socket)));
	UserMap[Socket]->IP = Address.sin_addr.S_un.S_addr;

	strcpy(UserMap[Socket]->IPchar, inet_ntoa(Address.sin_addr));
	sprintf(UserMap[Socket]->ClientSocket.info, "%d:%s", UserMap[Socket]->ClientSocket.Socket, UserMap[Socket]->IPchar);

	UserMapLock.WriteUnLock();
}

User& GetUser(SOCKET idx) {
	UserMapLock.ReadLock();
	User& myuser = *(UserMap.find(idx)->second);
	UserMapLock.ReadUnLock();
	return myuser;
}

void CompressArrays(SOCKET idx) {
	printf("delete idx : %d\n", idx);
	UserMapLock.WriteLock();
	IPMap[UserMap[idx]->IP]--;
	UserMap.erase(idx);
	UserMapLock.WriteUnLock();
}
void Compress(char *source, int len) {
	for (int k = 0; k < len; k++) 
		source[k + len] = source[k];
}



// 호출함수에서 lock 걸엇음.
void User::GetOthersInfo() {
	User* fromuser;
	char source[SOURCELEN];
	shared_ptr<Packet> ko;

	for (auto it = UserMap.begin();it != UserMap.end(); it++) {
		if (it->first != ClientSocket.Socket) {
			if (it->second->IsEnter == 1) {
				fromuser = it->second;
				int sum = 0;
				int namelen = strlen(fromuser->Name);

				DataAddCopy(source, &fromuser->CharacterClass, sizeof(char), sum);
				DataAddCopy(source, &fromuser->PosY, sizeof(float), sum);
				DataAddCopy(source, &fromuser->PosZ, sizeof(float), sum);
				DataAddCopy(source, &fromuser->Damage, sizeof(float), sum);
				DataAddCopy(source, fromuser->Name, namelen, sum);


				ko = make_shared<Packet>(PACKET_TYPE::PLAYERSPAWN, sum + FRONTLEN, it->first, source, BROADCAST_MODE::ONLYME);
				SentInfo temp(ko);
				PushAndSend(temp);
			}
		}
	}
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
	DataAddGet(&myuser.Kill, source, sizeof(unsigned int), sum);
	myuser.Name[len - sum - FRONTLEN] = '\0';
	DataAddGet(myuser.Name, source, len-sum-FRONTLEN, sum);
	memset(myuser.ClientSocket.info, 0, sizeof(myuser.ClientSocket.info));
	sprintf(myuser.ClientSocket.info, "%d:%s:%s", myuser.ClientSocket.Socket, myuser.IPchar, myuser.Name);


	printf("PosY : %f, PosZ : %f Dmg : %f Kill : %d\n", myuser.PosY, myuser.PosZ, myuser.Dmg, myuser.Kill);
	printf("username : %s\n", myuser.Name);

}


shared_ptr<Packet> NameCheckProcess(User& myuser, char * Name, int NameLen) {
	char ReqName[NAMELEN]{ 0 };
	char buf[1];
	bool isthere = false;
	bool flag = true;

	if (NameLen > 11) {
		printf("too long packet name");
		flag = false;
		buf[0] = (char)ERRORCODE::TOOLONG;
	}
	else {
		if (NameLen < 1) {
			printf("too short name");
			flag = false;
			buf[0] = (char)ERRORCODE::TOOSHORT;
		}
		else {
			memcpy(ReqName, Name, NameLen);
			for (int i = 0; i < strlen(ReqName); ++i)
			{
				if ('A' > ReqName[i] || ReqName[i] > 'z') {
					buf[0] = (char)ERRORCODE::NOTENGLISH;
					flag = false;
					break;
				}
			}
		}
	}
	
	if (flag == true) {
		UserMapLock.ReadLock();
		//EnterCriticalSection(&UserMapLock);

		for (auto it = UserMap.begin(); it != UserMap.end(); it++) {
			if (it->second->IsEnter == 1) {
				if (strcmp(ReqName, it->second->Name) == 0) {
					isthere = true;
					break;
				}
			}
		}
		UserMapLock.ReadUnLock();
		//LeaveCriticalSection(&UserMapLock);

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
}
void GetOut(SOCKET Socket) {
	char empty[1]{ 0 };
	shared_ptr<Packet> temppacket = make_shared<Packet>(PACKET_TYPE::LOGOUT, FRONTLEN + 1,Socket, empty, BROADCAST_MODE::EXCEPTME);
	SentInfo temp(temppacket);
	// broadcast
	EnterCriticalSection(&g_OrderQueueLock);
	g_OrderQueue.Push(temp);
	SetEvent(OrderQueueEvent);
	LeaveCriticalSection(&g_OrderQueueLock);


	//close socket
	printf("out! %d\n", Socket);
	closesocket(Socket);
	CompressArrays(Socket);
}

int RecvProcess(char * source, int retValue, User& myuser) {
	char * receivedBuffer = myuser.ClientSocket.ReceiveBuffer;
	int& receivedSize = myuser.ClientSocket.ReceivedBufferSize;

	memcpy(receivedBuffer + receivedSize, source, retValue);
	receivedSize += retValue;
	
	//logger.write("RecvProcess() before make packet, id:%d len:%d recvSize:%d retvalue:%d", myuser.ClientSocket.Socket, len, receivedSize, retValue);
	int len = 0;
	int sumlen = 0;

	while (receivedSize >= FRONTLEN && receivedSize >= (len = (int)*(wchar_t*)(receivedBuffer + sumlen))) {
		shared_ptr<Packet> temppacket;
		PACKET_TYPE Type = (PACKET_TYPE)*(receivedBuffer + USERLEN + LENLEN + sumlen);

		if (len > 50 || (unsigned char)Type > 6) {
			logger.write("[%s] not correct len type,len:%d recvSize:%d sumlen:%d", myuser.ClientSocket.info, len, receivedSize, sumlen);
			GetOut(myuser.ClientSocket.Socket);
			return -1;
		}
		
		char * Body = receivedBuffer + FRONTLEN + sumlen;
		
		receivedSize -= len;
		sumlen += len;

		logger.write("[%s] RecvProcess() after make packet, len:%d recvSize:%d sumlen:%d frontbuf:%c", myuser.ClientSocket.info, len, receivedSize, sumlen, *Body);
		switch (Type) {
		case PACKET_TYPE::PLAYERSPAWN:
			temppacket = make_shared<Packet>(Type, len, myuser.ClientSocket.Socket, Body, BROADCAST_MODE::ALL);
			SpawnProcess(myuser, temppacket, len);

			logger.write("[%s] PlayserSpawn", myuser.ClientSocket.info);
			break;
		case PACKET_TYPE::UPDATELOCATION:
			temppacket = make_shared<Packet>(Type, len, myuser.ClientSocket.Socket, Body, BROADCAST_MODE::EXCEPTME);
			myuser.PosY = *(float*)(Body);
			myuser.PosZ = *(float*)(Body + 4);
			myuser.Dir = *(Body + 8);

			logger.write("[%s] UpdataeLocation. PosY:%f PosZ:%f Dir:%d", myuser.ClientSocket.info, myuser.PosY, myuser.PosZ, myuser.Dir);
			break;
		case PACKET_TYPE::UPDATEDMG:
			temppacket = make_shared<Packet>(Type, len, myuser.ClientSocket.Socket, Body, BROADCAST_MODE::EXCEPTME);
			myuser.Damage = *(float*)Body;

			logger.write("[%s] UpdataeDmg. Dmg:%f", myuser.ClientSocket.info, myuser.Damage);
			break;
		case PACKET_TYPE::NAMECHECK:
			temppacket = NameCheckProcess(myuser, Body, len - FRONTLEN);

			logger.write("[%s] Namecheck", myuser.ClientSocket.info);
			break;
		case PACKET_TYPE::UPDATESTATE:
			temppacket = make_shared<Packet>(Type, len, myuser.ClientSocket.Socket, Body, BROADCAST_MODE::EXCEPTME);
			if (*Body == (char)ECharacterAction::EA_Die) {
				myuser.Kill = *(unsigned int*)(Body + 1);
				printf("%dkill!", myuser.Kill);
				myuser.IsEnter = 0;
				memset(myuser.Name, 0, NAMELEN);
			}

			logger.write("[%s] Updatestate. val:%d", myuser.ClientSocket.info, (int)*Body);
			break;
		}

		SentInfo temp(temppacket);

		EnterCriticalSection(&g_OrderQueueLock);
		g_OrderQueue.Push(temp);
		SetEvent(OrderQueueEvent);
		LeaveCriticalSection(&g_OrderQueueLock);


		//len = (int)*(wchar_t*)(receivedBuffer + sumlen);
		logger.write("[%s] RecvProcess() after(2) make packet, len:%d recvSize:%d sumlen:%d frontbuf:%c", myuser.ClientSocket.info, len, receivedSize, sumlen, *Body);
	}

	for (int k = 0; k < receivedSize; k++)
		receivedBuffer[k + sumlen] = receivedBuffer[k];

	return 1;
}
void Recver::Work(LPPER_HANDLE_DATA PerHandleData, DWORD bytes) {
	SOCKET Socket = PerHandleData->Socket;
	DWORD Flags;
	WSABUF wbuf;
	User& myuser = GetUser(Socket);
	clock_t end;

	end = clock();
	
	
	if ((double)(end - myuser.Begin) < 15.f) {
		if (++myuser.DDOS > 10) {
			GetOut(myuser.ClientSocket.Socket);
			return;
		}
	}
	else {
		if(myuser.DDOS > 0)
			myuser.DDOS--;
	}
	myuser.Begin = end;
	logger.write("[%s] Recver Work() bytes:%d frontbuf:%c", myuser.ClientSocket.info, bytes, *(Buffer+FRONTLEN));
	if (RecvProcess(Buffer, bytes, myuser) == -1)
		return;
	
	Flags = 0;
	BufferLen = MAX_BUFFER_SIZE;
	wbuf.buf = Buffer;
	wbuf.len = BufferLen;

	WSARecv(Socket, &wbuf, 1, NULL, &Flags, this, NULL);
}

void User::SendFront(Sender* overlapped) {
	WSABUF wBuf;
	EnterCriticalSection(&ClientSocket.WQL);
	
	if (ClientSocket.WaitingQueue.empty()) {
		logger.write("[%s] SendFront() empty()", ClientSocket.info);
		LeaveCriticalSection(&ClientSocket.WQL);
		return;
	}
	
	SentInfo &target = ClientSocket.WaitingQueue.Front();

	LeaveCriticalSection(&ClientSocket.WQL);

	logger.write("[%s] SendFront() sended:%d frontbuf:%c", ClientSocket.info, target.Sended, *(target.Sp.get()->Body + FRONTLEN));
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

	logger.write("[%s] Send Work() bytes:%d frontbuf:%c", myuser.ClientSocket.info, bytes, *(sendinfo.Sp.get()->Body + FRONTLEN));

	if (sendinfo.Sended == sendinfo.MaxLen) {
		if (myuser.ClientSocket.WaitingQueue.empty()) {
			logger.write("[%s] Send Work() empty() frontbuf:%c", myuser.ClientSocket.info, *(sendinfo.Sp.get()->Body + FRONTLEN));
			LeaveCriticalSection(&myuser.ClientSocket.WQL);
			return;
		}
		else {
			logger.write("[%s] Send Work() !empty() frontbuf:%c", myuser.ClientSocket.info,*(sendinfo.Sp.get()->Body + FRONTLEN));
			EnterCriticalSection(&myuser.ClientSocket.WQL);
			myuser.ClientSocket.WaitingQueue.Pop();
			if (myuser.ClientSocket.WaitingQueue.empty()) {
				logger.write("[%s] Send Work() Pop and empty() frontbuf:%c", myuser.ClientSocket.info, *(sendinfo.Sp.get()->Body + FRONTLEN));

				LeaveCriticalSection(&myuser.ClientSocket.WQL);
				return;
			}
			else {
				logger.write("[%s] Send Work() Pop and !empy() frontbuf:%c", myuser.ClientSocket.info, *(sendinfo.Sp.get()->Body + FRONTLEN));

				SentInfo &target = myuser.ClientSocket.WaitingQueue.Front();

				wBuf.buf = target.Sp.get()->Body;
				wBuf.len = target.MaxLen;
				sendinfo = target;

				WSASend(myuser.ClientSocket.Socket, &wBuf, 1, NULL, 0, this, NULL);
				LeaveCriticalSection(&myuser.ClientSocket.WQL);

			}
		}
		
	}
	else if(sendinfo.Sended < sendinfo.MaxLen)
	{
		logger.write("[%s] Send Work() sended < maxlen frontbuf:%c", myuser.ClientSocket.info, *(sendinfo.Sp.get()->Body + FRONTLEN));

		wBuf.buf = sendinfo.Sp.get()->Body + sendinfo.Sended;
		wBuf.len = sendinfo.MaxLen - sendinfo.Sended;
		
		WSASend(myuser.ClientSocket.Socket, &wBuf, 1, NULL, 0, this, NULL);
	}
	else {
		logger.write("[%s] Sendwork() exception", myuser.ClientSocket.info);

		return; 
	}
	logger.write("[%s] End Send Work()", myuser.ClientSocket.info);

}

void User::PushAndSend(SentInfo& temp) {
	EnterCriticalSection(&ClientSocket.WQL);
	if (ClientSocket.WaitingQueue.empty()) {
		logger.write("[%s] PushAndSend empty() frontbuf : %c", ClientSocket.info, *(temp.Sp.get()->Body + FRONTLEN));

		ClientSocket.WaitingQueue.Push(temp);
		LeaveCriticalSection(&ClientSocket.WQL);


		Sender* PerIoData = new Sender();
		SendFront(PerIoData);
	}
	else
	{
		logger.write("[%s] PushAndSend() !empty() frontbuf : %c", ClientSocket.info, *(temp.Sp.get()->Body + FRONTLEN));


		ClientSocket.WaitingQueue.Push(temp);
		LeaveCriticalSection(&ClientSocket.WQL);

	}

}