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

enum class PACKET_TYPE {
	ENTER,
	PLAYERSPAWN,
	UPDATETRANSFORM,
	UPDATEDMG,
	UPDATESTATE,
};

class Packet {
public:
	PACKET_TYPE type;
	char userID;
	uint8 body[MAX_PACKET_SIZE];

	uint32 remain;
	uint32 len;

	Packet(PACKET_TYPE myType, uint8 myId, uint8* myBody, uint32 myLen) {
		type = myType;
		userID = myId;
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

	// Tick 델리게이트
	FDelegateHandle TickDelegateHandle;

	class FSocket* ConnectionSocket;
	uint8 BufArray[10000];
	uint8 * targetArray;
	uint32 BufArraySize;

	bool bIsConnected;

	uint8 ReadData[BUFLEN];
	uint32 Size;
	int32 ReadBytes;
	uint32 CurrentUserNum;
	uint32 sumLen;
	uint32 Len;
	char MyID;

	UPROPERTY(EditAnywhere)
	class ACharacterSpawner* CharacterSpawner;
	
	class APlayerCharacter* PlayerList[100];
	class APlayerCharacter* MyCharacter;
	class APlayerController* PlayerController;
	FVector NewPosition;

	// 패킷 전송 함수
	void SendMessage(PACKET_TYPE Type, char * Body, wchar_t size);
	// 들어온 패킷 분석
	void PacketHandler();
	// 들어온 패킷 처리
	void PacketProcess(Packet& packet);

	// 플레이어 입장
	UFUNCTION(BlueprintCallable)
	void EnterGame(FString ServerIP, int32 ServerPort, FString UserName, uint8 ClassType);
};
