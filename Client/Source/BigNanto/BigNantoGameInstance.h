// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BigNantoGameInstance.generated.h"

#define BUFLEN 512
#define MAX_PACKET_SIZE 3000
#define NAMELEN 20

#define LENLEN 2
#define USERLEN 4
#define TYPELEN 1
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
enum class ERRORCODE
{
	NOTERROR,
	TOOLONG,
	ALREADYNAME,
	NOTENGLISH,
	TOOSHORT,
};

class Packet {
public:
	PACKET_TYPE type;
	uint32 userID;
	uint8 body[MAX_PACKET_SIZE];

	uint32 remain;
	uint32 len;

	Packet(PACKET_TYPE myType, uint32 myId, uint8* myBody, uint32 myLen) {
		type = myType;
		userID = myId;
		len = myLen;

		FMemory::Memcpy(body, myBody, len - FRONTLEN);
		body[len - FRONTLEN] = 0;
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

	uint8 ReadData[BUFLEN];
	uint32 Size;
	int32 ReadBytes;
	uint32 CurrentUserNum;
	uint32 sumLen;
	uint32 Len;

	// 내 유저 정보
	uint32 MyID;			// 아이디

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MyName;		// 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 MyClassType;	// 직업 명

	UPROPERTY()
	class ACharacterSpawner* CharacterSpawner;
	UPROPERTY()
	class ACenterViewPawn* CenterViewPawn;
	
	TMap<uint32, class APlayerCharacter*> PlayerList;
	class ABigNantoPlayerController* PlayerController;
	class ABigNantoGameModeBase* GameModeBase;
	FVector NewPosition;

	// 패킷 전송 함수
	void SendMessage(PACKET_TYPE Type, char * Body, uint32 size);
	// 들어온 패킷 분석
	void PacketHandler();
	// 들어온 패킷 처리
	void PacketProcess(Packet& packet);

	// 로그인 버퍼 생성 함수
	int MakeLoginBuf(char * source, char cls, float Y, float Z, float damage, uint32 KillCount, char * name, int namelen);
	// 플레이어 입장
	UFUNCTION(BlueprintCallable)
	bool EnterGame(FString ServerIP, int32 ServerPort);

	UFUNCTION(BlueprintCallable)
	void NameCheck(FString UserName, uint8 ClassType);

	// 패킷 만들때 편하게
	template <typename T>
	void DataAddCopy(char * source, T* get, int size, int& sum);

	template <typename T>
	void DataAddGet(T* source, char* get, int size, int& sum);

};


