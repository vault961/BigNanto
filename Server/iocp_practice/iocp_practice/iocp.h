#pragma once

#include <stdio.h>
#include <winsock2.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <queue>
#pragma comment (lib, "Ws2_32.lib")


#define MAX_BUFFER_SIZE 512
#define MAX_PACKET_SIZE 3000
#define MAX_SOCKET_BUFFER_SIZE 3000
#define MAX_USER 10
#define TIMESTAMPLEN 8

#define RECV_POSTED 1
#define SENDER 2
#define ORDER 3
#define SEND_POSTED 4

#define TYPELEN 1
#define USERLEN 1
#define LENLEN 2
#define FRONTLEN TYPELEN+USERLEN+LENLEN

enum class PACKET_TYPE {
	ENTER,
	LOGIN,
	UPDATEPOS,
	UPDATESTATUS,
};
typedef struct _PER_HANDLE_DATA {
	SOCKET Socket;
	int idx;
	SOCKADDR_IN ClientAddr;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;


class Packet{
public:
	PACKET_TYPE Type;
	char UserID;
	wchar_t Len;
	time_t Timestamp;
	char Body[MAX_PACKET_SIZE];
	Packet(PACKET_TYPE myType, wchar_t myLen, int myIdx, char * myBody) {
		Type = myType;
		UserID = myIdx;
		Len = myLen + TIMESTAMPLEN;
		memcpy(Body + FRONTLEN, myBody, myLen - FRONTLEN);
		time(&Timestamp);
	}
};



class RWLock
{
public:
	RWLock() {};
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
	char * BufRef;
	
	SentInfo() {

	}
	SentInfo(Packet& source) {
		memcpy(source.Body, &source.Type, TYPELEN);
		memcpy(source.Body + TYPELEN, &source.UserID, USERLEN);
		memcpy(source.Body + TYPELEN + USERLEN, &source.Len, LENLEN);
		memcpy(source.Body + source.Len - TIMESTAMPLEN, &source.Timestamp, TIMESTAMPLEN);

		BufRef = source.Body;
		Sended = 0;
		MaxLen = source.Len;
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
	virtual void Work(SOCKET Socket, int idx, DWORD bytes) = 0;

};

class Sender : public Worker
{
public:
	virtual void Work(SOCKET Socket, int idx, DWORD bytes);
	SentInfo sendinfo;
};
class Recver : public Worker
{
public:
	virtual void Work(SOCKET Socket, int idx, DWORD bytes);
	char Buffer[MAX_BUFFER_SIZE]{ 0 };
	int BufferLen{ 0 };
};

typedef struct {
	int Idx;
	ClientSocket ClientSocket;
	BOOL Connection;
	bool Sending;
	char Name[30];
	char Class;
	int Damage;
	float PosY;
	float PosZ;
	char Kind;

	void SendFront(Sender* overlapped);
	void GetOthersInfo();
	RWLock wqueue;
} User;


extern OrderQueue<Packet> g_OrderQueue;
extern User UserList[MAX_USER];
extern WSAEVENT OrderQueueEvent;
extern std::vector<int> UserVec;
extern RWLock UserVecLock;
extern RWLock UserListLock;
extern SOCKET ListenSocket;
extern HANDLE CompletionPort;


User& GetUser(int idx);
void CompressArrays(int i);
int AddSocket(int Socket);
void Compress(char *source, int len);
void CreateAccount(Packet *temppacket, char* name, char* buf, int len);
void RecvProcess(char * source, int retValue, User& myuser);
void OrderQueueThread();

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

