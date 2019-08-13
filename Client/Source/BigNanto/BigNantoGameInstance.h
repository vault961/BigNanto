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

	// Tick ��������Ʈ
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

	// �� ���� ����
	uint32 MyID;			// ���̵�

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MyName;		// �̸�
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 MyClassType;	// ���� ��

	UPROPERTY()
	class ACharacterSpawner* CharacterSpawner;
	UPROPERTY()
	class ACenterViewPawn* CenterViewPawn;
	
	TMap<uint32, class APlayerCharacter*> PlayerList;
	class ABigNantoPlayerController* PlayerController;
	class ABigNantoGameModeBase* GameModeBase;
	FVector NewPosition;

	// ��Ŷ ���� �Լ�
	void SendMessage(PACKET_TYPE Type, char * Body, uint32 size);
	// ���� ��Ŷ �м�
	void PacketHandler();
	// ���� ��Ŷ ó��
	void PacketProcess(Packet& packet);

	// �α��� ���� ���� �Լ�
	int MakeLoginBuf(char * source, char cls, float Y, float Z, float damage, char * name, int namelen);
	// �÷��̾� ����
	UFUNCTION(BlueprintCallable)
	bool EnterGame(FString ServerIP, int32 ServerPort);

	UFUNCTION(BlueprintCallable)
	void NameCheck(FString UserName, uint8 ClassType);

	// ��Ŷ ���鶧 ���ϰ�
	template <typename T>
	void DataAddCopy(char * source, T* get, int size, int& sum);

	template <typename T>
	void DataAddGet(T* source, char* get, int size, int& sum);

};


