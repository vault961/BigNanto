// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BigNantoGameInstance.generated.h"

#define BUFLEN 512
#define MAX_PACKET_SIZE 3000
#define NAMELEN 20
#define USERLEN 1
#define TYPELEN 1
#define FRONTLEN 4
#define TIMESTAMPLEN 8

enum class PACKET_TYPE {
	ENTER,
	PLAYERSPAWN,
	UPDATEDATA,
};

class Packet {
public:
	PACKET_TYPE type;
	char userID;
	time_t timestamp;
	uint8 body[MAX_PACKET_SIZE];

	uint32 remain;
	uint32 len;

	Packet(PACKET_TYPE myType, uint8 myId, uint8* myBody, uint32 myLen) {
		type = myType;
		userID = myId;
		//	timestamp = myTimeStamp;
		len = myLen;
		memcpy(body, myBody, len - 12);
		body[len - 12] = 0;
	}
};

UCLASS()
class BIGNANTO_API UBigNantoGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UBigNantoGameInstance();
	virtual void Init() override;
	virtual bool Tick(float DeltaTime);
	virtual void Shutdown() override;

	FDelegateHandle TickDelegateHandle;

	class FSocket* ConnectionSocket;
	uint8 BufArray[10000];
	uint8 * targetArray;
	uint32 BufArraySize;

	uint8 ReadData[BUFLEN];
	uint32 Size;
	int32 ReadBytes;
	uint32 CurrentUserNum;
	uint32 sumLen;
	uint32 Len;
	char MyID;

	UPROPERTY(EditAnywhere)
	class ACharacterSpawner* CharacterSpawner;
	
	//class PlayerManager* PlayerManager;

	class APlayerCharacter* PlayerList[100];
	class APlayerCharacter* MyCharacter;
	class APlayerController* PlayerController;
	FVector NewPosition;

	void PacketHandler();
	void SendMessage(PACKET_TYPE Type, char * Body, wchar_t size);
	void PacketProcess(Packet& packet);
};
