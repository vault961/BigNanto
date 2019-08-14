#pragma once

#include <stdio.h>
#include <winsock2.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <queue>
#include <map>
#include <memory>
#include <functional>
#pragma comment (lib, "Ws2_32.lib")


#define MAX_BUFFER_SIZE 512
#define MAX_PACKET_SIZE 512
#define MAX_SOCKET_BUFFER_SIZE 10000
#define MAX_USER 64

#define RECV_POSTED 1
#define SENDER 2
#define ORDER 3
#define SEND_POSTED 4

#define SOURCELEN 50
#define NAMELEN 50

#define TYPELEN 1
#define USERLEN 4
#define LENLEN 2
#define FRONTLEN 7



enum class PACKET_TYPE {
	ENTER,
	PLAYERSPAWN,
	UPDATELOCATION,
	UPDATEDMG,
	UPDATESTATE,
	NAMECHECK,
	LOGOUT,
	KILL,
};
enum class ECharacterAction
{
	EA_DefendHit,
	EA_StopAttack,
	EA_Attack,
	EA_Defend,
	EA_Hit,
	EA_Jump,
	EA_SpecialAbility,
	EA_StopSpecialAbility,
	EA_Die,
	EA_Move,
	EA_StopMove,

};
enum class ERRORCODE
{
	NOTERROR,
	TOOLONG,
	ALREADYNAME,
	NOTENGLISH,
	TOOSHORT,
};

enum class BROADCAST_MODE {
	ALL,
	EXCEPTME,
	ONLYME,
};

typedef struct _PER_HANDLE_DATA {
	SOCKET Socket;
	SOCKADDR_IN ClientAddr;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;


class Packet{
public:
	PACKET_TYPE Type;
	SOCKET UserID;
	wchar_t Len;
	char Body[MAX_PACKET_SIZE];
	BROADCAST_MODE BMode;

	Packet(PACKET_TYPE myType, wchar_t myLen, SOCKET myIdx, char * myBody, BROADCAST_MODE Mode) {
		Type = myType;
		UserID = myIdx;
		Len = myLen;
		BMode = Mode;
		memcpy(Body + FRONTLEN, myBody, myLen - FRONTLEN);
		memcpy(Body + LENLEN + USERLEN, &Type, TYPELEN);
		memcpy(Body + LENLEN, &UserID, USERLEN);
		memcpy(Body, &Len, LENLEN);

	}
	~Packet() {

	}
};



class RWLock
{
public:
	RWLock() {
		mLockFlag = 0;
	};
	~RWLock() {};

	RWLock(const RWLock& rhs) = delete;
	RWLock& operator=(const RWLock& rhs) = delete;

	/// exclusive mode
	void WriteLock();
	void WriteUnLock();

	/// shared mode
	void ReadLock();
	void ReadUnLock();

	long GetLockFlag() const { return mLockFlag; }
private:
	enum LockFlag
	{
		LF_WRITE_MASK = 0x7FF00000,
		LF_WRITE_FLAG = 0x00100000,
		LF_READ_MASK = 0x000FFFFF
	};
	volatile long mLockFlag;
};




template <typename T>
class OrderQueue {
public:
	OrderQueue() {
	}
	void Push(T&);
	bool empty();
	T& Pop();
	T& Front();
	int Size();
private:
	std::queue<T> queue;
	//RWLock lock;
};



class SentInfo {
public:
	unsigned int Sended;
	unsigned int MaxLen;
	std::shared_ptr<Packet> Sp;
	BROADCAST_MODE BMode;
	SentInfo() {

	}
	SentInfo(std::shared_ptr<Packet> Sr) {
		Sp = Sr;
		Sended = 0;
		MaxLen = Sp.get()->Len;
		BMode = Sp.get()->BMode;
	}
};

class ClientSocket{
public:
	SOCKET Socket;
	char ReceiveBuffer[MAX_SOCKET_BUFFER_SIZE];
	int ReceivedBufferSize;
	char info[100]{ 0 };
	OrderQueue<SentInfo> WaitingQueue;
	CRITICAL_SECTION WQL;

	ClientSocket() {
		InitializeCriticalSection(&WQL);
	}
	~ClientSocket() {
		DeleteCriticalSection(&WQL);
	}
};




class Worker : public OVERLAPPED
{
protected:
	Worker() {
		Internal = 0;
		InternalHigh = 0;
		hEvent = 0;
		OffsetHigh = 0;
		Offset = 0;
		Pointer = 0;
	}
public:
	virtual void Work(LPPER_HANDLE_DATA PerHandleData, DWORD bytes) = 0;
};

class Sender : public Worker
{
public:
	virtual void Work(LPPER_HANDLE_DATA PerHandleData, DWORD bytes);
	SentInfo sendinfo;
};
class Recver : public Worker
{
public:
	virtual void Work(LPPER_HANDLE_DATA PerHandleData, DWORD bytes);
	char Buffer[MAX_BUFFER_SIZE]{ 0 };
	int BufferLen{ 0 };
};



class Log {
public:
	FILE *fp;
	CRITICAL_SECTION cri;
	Log() {
		SYSTEMTIME cur_time;
		GetLocalTime(&cur_time);
		char name[NAMELEN];
		InitializeCriticalSection(&cri);

		sprintf(name, "[%02d%02d%02d]log.txt", cur_time.wHour, cur_time.wMinute, cur_time.wSecond);
		fp = fopen(name, "a");
	}
	
	void write(char *format, ...) {
		EnterCriticalSection(&cri);
		SYSTEMTIME cur_time;
		GetLocalTime(&cur_time);


		va_list args;
		va_start(args, format);
		fprintf(fp, "[%02d:%02d:%02d.%03d]", cur_time.wHour, cur_time.wMinute, cur_time.wSecond, cur_time.wMilliseconds);
		vfprintf(fp, format, args);
		fprintf(fp, "\n");
		va_end(args);
		LeaveCriticalSection(&cri);

	}
	~Log() {
		fclose(fp);
		DeleteCriticalSection(&cri);

	}
};

class User {
public:
	User(SOCKET socket) {
		IsEnter = 0;
		ClientSocket.Socket = socket;
		CharacterClass = 0;
		PosY = 0;
		PosZ = 0;
		Damage = 0;
		Dir = 0;
		Kill = 0;
		ClientSocket.ReceivedBufferSize = 0;
		Begin = clock();
		DDOS = 0;
		sprintf(ClientSocket.info, "%d:", socket);
	}
	ClientSocket ClientSocket;
	clock_t Begin;
	ULONG IP;

	unsigned char DDOS;

	char IPchar[30]{ 0 };

	char IsEnter;
	char Name[NAMELEN]{ 0 };
	char CharacterClass;
	float Damage;
	float PosY;
	float PosZ;
	unsigned int Kill;
	char Dir;

	void SendFront(Sender* overlapped);
	void PushAndSend(SentInfo& temp);
	void GetOthersInfo();
};


extern OrderQueue<SentInfo> g_OrderQueue;
extern RWLock UserMapLock;
extern SOCKET ListenSocket;
extern HANDLE CompletionPort;
extern std::map<SOCKET, User*> UserMap;
extern Log logger;
extern std::map<ULONG, int> IPMap;
extern CRITICAL_SECTION g_OrderQueueLock;

User& GetUser(SOCKET idx);
void CompressArrays(SOCKET i);
void AddSocket(SOCKET Socket, SOCKADDR_IN Address);
void Compress(char *source, int len);
int RecvProcess(char * source, int retValue, User& myuser);
void OrderQueueThread();
void SpawnProcess(User& myuser, std::shared_ptr<Packet>& temppacket, int& len);
void GetOut(SOCKET Socket);


template <typename T>
void DataAddCopy(char * source, T* get, int size, int& sum);
template <typename T>
void DataAddGet(T* source, char* get, int size, int& sum);

template <typename T>
bool OrderQueue<T>::empty() {
	bool val;
	val = queue.empty();
	return val;
}

template <typename T>
void OrderQueue<T>::Push(T& packet) {
	queue.push(packet);
	return;
}

template <typename T>
T& OrderQueue<T>::Pop() {
	T& front = queue.front();
	queue.pop();
	return front;
}

template <typename T>
T& OrderQueue<T>::Front() {
	T& front = queue.front();
	return front;
}

template <typename T>
int OrderQueue<T>::Size() {
	return queue.size();
}
