#pragma once

#include <stdio.h>
#include <winsock2.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <queue>
#include <unordered_map>
#include <memory>
#include <functional>
#pragma comment (lib, "Ws2_32.lib")


#define MAX_BUFFER_SIZE 512
#define MAX_PACKET_SIZE 512
#define MAX_SOCKET_BUFFER_SIZE 10000
#define MAX_USER 10

#define RECV_POSTED 1
#define SENDER 2
#define ORDER 3
#define SEND_POSTED 4

#define TYPELEN 1
#define USERLEN 1
#define LENLEN 2
#define FRONTLEN 4



enum class PACKET_TYPE {
	ENTER,
	PLAYERSPAWN,
	UPDATEDATA,
};
typedef struct _PER_HANDLE_DATA {
	SOCKET Socket;
	char idx;
	SOCKADDR_IN ClientAddr;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;


class Packet{
public:
	PACKET_TYPE Type;
	char UserID;
	wchar_t Len;
	char Body[MAX_PACKET_SIZE];
	Packet(PACKET_TYPE myType, wchar_t myLen, char myIdx, char * myBody) {
		Type = myType;
		UserID = myIdx;
		Len = myLen;
		memcpy(Body + FRONTLEN, myBody, myLen - FRONTLEN);
		memcpy(Body, &Type, TYPELEN);
		memcpy(Body + TYPELEN, &UserID, USERLEN);
		memcpy(Body + TYPELEN + USERLEN, &Len, LENLEN);

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
private:
	std::queue<T> queue;
	RWLock lock;
};



class SentInfo {
public:
	unsigned int Sended;
	unsigned int MaxLen;
	std::shared_ptr<Packet> Sp;

	SentInfo() {

	}
	SentInfo(std::shared_ptr<Packet> Sr) {
		Sp = Sr;
		Sended = 0;
		MaxLen = Sp.get()->Len;
	}
};

typedef struct {
	SOCKET Socket;
	char ReceiveBuffer[MAX_SOCKET_BUFFER_SIZE];
	int ReceivedBufferSize;
	OrderQueue<SentInfo> WaitingQueue;
} ClientSocket;




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



class User {
public:
	User(char idx, int socket) {
		Idx = idx;
		ClientSocket.Socket = socket;
		CharacterClass = 0;
		PosY = 0;
		PosZ = 0;
		Damage = 0;
		ClientSocket.ReceivedBufferSize = 0;
	}
	char Idx;
	ClientSocket ClientSocket;
	char Name[30];
	char CharacterClass;
	int Damage;
	float PosY;
	float PosZ;
	
	void SendFront(Sender* overlapped);
	void GetOthersInfo();
	RWLock wqueue;
};


extern OrderQueue<SentInfo> g_OrderQueue;
extern RWLock UserMapLock;
extern SOCKET ListenSocket;
extern HANDLE CompletionPort;
extern std::unordered_map<char, User*> UserMap;

User& GetUser(char idx);
void CompressArrays(char i);
char AddSocket(int Socket);
void Compress(char *source, int len);
void RecvProcess(char * source, int retValue, User& myuser);
void OrderQueueThread();
void SpawnProcess(User& myuser, std::shared_ptr<Packet>& temppacket, wchar_t& len);
void EnterProcess(User& myuser, std::shared_ptr<Packet>& temppacket);


template <typename T>
bool OrderQueue<T>::empty() {
	bool val;
	lock.ReadLock();
	val = queue.empty();
	lock.ReadUnLock();
	return val;
}

template <typename T>
void OrderQueue<T>::Push(T& packet) {
	lock.WriteLock();
	queue.push(packet);
	lock.WriteUnLock();
	SetEvent(OrderQueueEvent);
}

template <typename T>
T& OrderQueue<T>::Pop() {
	lock.WriteLock();
	T& front = queue.front();
	queue.pop();
	lock.WriteUnLock();

	return front;
}

template <typename T>
T& OrderQueue<T>::Front() {
	lock.ReadLock();
	T& front = queue.front();
	lock.ReadUnLock();

	return front;
}

