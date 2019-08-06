// Fill out your copyright notice in the Description page of Project Settings.


#include "BigNantoGameInstance.h"
#include "Engine/Engine.h"
#include "Sockets/Public/Sockets.h"
#include "Sockets/Public/IPAddress.h"
#include "Networking/Public/Networking.h"
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "CharacterSpawner.h"
#include "UObject/ConstructorHelpers.h"
#include "PlayerCharacterAnim.h"
#include "CenterViewCamera.h"
#include "BigNantoPlayerController.h"

//#pragma comment (lib, "Ws2_32.lib")

UBigNantoGameInstance::UBigNantoGameInstance()
{
	NewPosition.X = 0;
	BufArraySize = 0;
	CurrentUserNum = 0;
	sumLen = 0;
}

void UBigNantoGameInstance::Init()
{
	UE_LOG(LogTemp, Log, TEXT("GameInstance Initiate"));
}

bool UBigNantoGameInstance::Tick(float DeltaTime)
{
	PacketHandler();
	return true;
}

void UBigNantoGameInstance::Shutdown()
{
	UE_LOG(LogTemp, Log, TEXT("GameInstance Shutdown..........."));
	FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
	if(nullptr != ConnectionSocket)
		ConnectionSocket->Close();
}

void UBigNantoGameInstance::SendMessage(PACKET_TYPE Type, char * Body, wchar_t BodySize)
{
	uint32 size = (uint32)BodySize + FRONTLEN;
	int32 sent = 0;

	char BUF[BUFLEN];

	memcpy(BUF + LENLEN + USERLEN, &Type, TYPELEN);
	memcpy(BUF, &size, LENLEN);
	memcpy(BUF + FRONTLEN, Body, BodySize);

	do {
		bool successful = ConnectionSocket->Send((uint8*)BUF, size, sent);

		if (!successful) {
			UE_LOG(LogTemp, Error, TEXT("Message can't send!!!!!!!!"));
		}
		size -= sent;

	} while (size > 0);
	return;
}

template <typename T>
void UBigNantoGameInstance::DataAddCopy(char * source, T* get, int size, int& sum) {
	memcpy(source + sum, get, size);
	sum += size;
}
template <typename T>
void UBigNantoGameInstance::DataAddGet(T* source, char* get, int size, int& sum) {
	memcpy(source, get + sum, size);
	sum += size;
}


void UBigNantoGameInstance::PacketHandler()
{
	if (ConnectionSocket->HasPendingData(Size)) {
		ReadBytes = 0;
		ConnectionSocket->Recv(ReadData, BUFLEN, ReadBytes, ESocketReceiveFlags::None);
		if (ReadBytes == 0) // error
			return;
		if (BufArraySize + ReadBytes < MAX_PACKET_SIZE) {
			memcpy(BufArray + BufArraySize, ReadData, ReadBytes);
			BufArraySize += ReadBytes;
		}
	}
	// 배열 이동을 최소화하기위해 쌓인버퍼 모두 처리후 배열이동.
	sumLen = 0;
	while (BufArraySize > 0) {
		targetArray = BufArray + sumLen;

		if (BufArraySize < FRONTLEN)
			break;

		Len = *(wchar_t*)targetArray;
		if (BufArraySize >= Len) {

			Packet packet((PACKET_TYPE)*(char*)(targetArray+ LENLEN + USERLEN), *(uint32*)(targetArray + LENLEN), targetArray + FRONTLEN, Len);

			PacketProcess(packet);
			sumLen += Len;
			BufArraySize -= Len;
		}
		else {
			break;
		}
	}
	for (uint32 i = 0; i < BufArraySize; i++) {
		BufArray[BufArraySize + i] = BufArray[i];
	}
}

void UBigNantoGameInstance::PacketProcess(Packet& packet) 
{
	FVector none;

	switch (packet.type) {
	case PACKET_TYPE::ENTER:
	{
		// 내 ID 받기
		MyID = packet.userID;

		// 랜덤한 위치에 소환하기 위해 랜덤벡터 생성
		FVector RandomLocation = CharacterSpawner->GetRandomPointInVolume();

		// 직업타임, Y좌표, Z좌표, 데미지 퍼센트, 이름 PlayerSpawn 타입 패킷으로 전송
		char buf[30];
		
		int len = MakeLoginBuf(buf, MyClassType, RandomLocation.Y, RandomLocation.Z, 0, TCHAR_TO_ANSI(*MyName), MyName.Len());
		UE_LOG(LogTemp, Warning, TEXT("%s"), MyName.GetCharArray().GetData());

		// 서버에게 내 캐릭터 요청
		SendMessage(PACKET_TYPE::PLAYERSPAWN, buf, len);
		break;
	}
	case PACKET_TYPE::PLAYERSPAWN:
	{
		int sum = 0;
		char * source = (char*)packet.body;
		
		char CharacterClass;
		float PosY;
		float PosZ;
		float DamagePercent;
		uint8 PlayerName[30];

		DataAddGet(&CharacterClass, source, sizeof(char), sum);
		DataAddGet(&PosY, source, sizeof(float), sum);
		DataAddGet(&PosZ, source, sizeof(float), sum);
		DataAddGet(&DamagePercent, source, sizeof(float), sum);

		int NameLen = packet.len - sum - FRONTLEN;
		DataAddGet(PlayerName, source, NameLen, sum);

		// 패킷 ID와 내 ID 비교
		// 내 캐릭터 스폰
		if (packet.userID == MyID)
		{
			MyCharacter = CharacterSpawner->SpawnCharacter(CharacterClass, PosY, PosZ, DamagePercent, true);
			if (nullptr == MyCharacter)
			{
				UE_LOG(LogTemp, Error, TEXT("내 캐릭터를 정상적으로 스폰할 수 없습니다"));
				return;
			}
			PlayerList[packet.userID] = MyCharacter;

			memcpy(MyCharacter->Name, PlayerName, NameLen);
			MyCharacter->Name[NameLen] = '\0';

			char NameArray[20];
			memcpy(NameArray, PlayerName, NameLen);
			MyCharacter->PlayerName = FString(UTF8_TO_TCHAR(NameArray));
			UE_LOG(LogTemp, Warning, TEXT("GameInstance Name = %s"), NameArray);
			UE_LOG(LogTemp, Warning, TEXT("CharacterFString = %s"), *MyCharacter->PlayerName);
			UE_LOG(LogTemp, Warning, TEXT("CharacterArrayName = %s"), MyCharacter->Name);

			MyCharacter->MyID = MyID;

			if (nullptr == PlayerController)
				PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
			PlayerController->Possess(MyCharacter);

			CurrentUserNum++;
			UE_LOG(LogTemp, Warning, TEXT("myuser login"));
		}
		// 다른 캐릭터들 스폰
		else
		{
			APlayerCharacter* Character = CharacterSpawner->SpawnCharacter(CharacterClass, PosY, PosZ, DamagePercent, false);
			if (nullptr == Character)
			{
				UE_LOG(LogTemp, Error, TEXT("내 캐릭터를 정상적으로 스폰할 수 없습니다"));
				return;
			}
			//float movespeed = Character->GetVelocity().Size();
			PlayerList[packet.userID] = Character;

			memcpy(Character->Name, PlayerName, NameLen);
			Character->Name[NameLen] = '\0';

			CurrentUserNum++;
			UE_LOG(LogTemp, Warning, TEXT("other user login"));
		}
		break;
	}
	case PACKET_TYPE::UPDATELOCATION:
	{
		APlayerCharacter* User = PlayerList[packet.userID];
		NewPosition.Y = *(float*)packet.body;
		NewPosition.Z = *(float*)(packet.body + 4);
		User->NewDir = *(packet.body + 8);
		User->UpdateLocation(NewPosition);
		break;
	}
	case PACKET_TYPE::UPDATEDMG:
	{
		APlayerCharacter* User = PlayerList[packet.userID];
		User->DamagePercent = *(float*)(packet.body);
		break;
	}
	case PACKET_TYPE::UPDATESTATE:
	{
		if (packet.userID != MyID) {
			APlayerCharacter* User = PlayerList[packet.userID];
			switch (packet.body[0]) {
			case (char)ECharacterAction::EA_Attack:
				User->Attack();
				break;
			case (char)ECharacterAction::EA_Defend:
				User->DoJump();
				break;
			case (char)ECharacterAction::EA_DefendHit:
				User->AnimInstance->PlayDefendHit();
				break;
			case (char)ECharacterAction::EA_Hit:
				User->HitandKnockback(none, 0);
				break;
			case (char)ECharacterAction::EA_Jump:
				break;
			case (char)ECharacterAction::EA_StopAttack:
				User->StopAttack();
				break;
			}
		}
	}
	
	}
}

int UBigNantoGameInstance::MakeLoginBuf(char * source, char cls, float Y, float Z, float damage, char * name, int namelen) {
	int sum = 0;
	DataAddCopy(source, &cls, sizeof(char), sum);
	DataAddCopy(source, &Y, sizeof(float), sum);
	DataAddCopy(source, &Z, sizeof(float), sum);
	DataAddCopy(source, &damage, sizeof(float), sum);
	DataAddCopy(source, name, namelen, sum);
	return sum;
}

void UBigNantoGameInstance::EnterGame(FString ServerIP, int32 ServerPort, FString UserName, uint8 ClassType)
{
	ConnectionSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("TCP"), false);
	if (!ConnectionSocket) {
		UE_LOG(LogTemp, Error, TEXT("Cannot create socket."));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("PlayerController Name : %s"), *PlayerController->GetName());

	UE_LOG(LogTemp, Log, TEXT("ServerIP = %s"), *ServerIP);
	UE_LOG(LogTemp, Log, TEXT("ServerPort = %d"), ServerPort);
	UE_LOG(LogTemp, Log, TEXT("UserName = %s"), *UserName);
	UE_LOG(LogTemp, Log, TEXT("ClassType = %d"), ClassType);

	MyName = UserName + '\0';
	MyClassType = ClassType;

	TSharedRef<FInternetAddr> RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	FIPv4Address address;
	FIPv4Address::Parse(ServerIP, address); 
	RemoteAddress->SetIp(address.Value);	// 서버 아이피 세팅
	RemoteAddress->SetPort(ServerPort);		// 서버 포트 세팅
	
	// 소켓 연결 여부 확인
	if (ConnectionSocket->Connect(*RemoteAddress))
	{
		UE_LOG(LogTemp, Warning, TEXT("Connection done."));

		// 틱 돌아가기 시작
		TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UBigNantoGameInstance::Tick));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, FString::Printf(TEXT("Connection Failed")));
		return;
	}

	// 게임 입장 패킷 전송
	char empty[1] = "" ;
	SendMessage(PACKET_TYPE::ENTER, empty, 1);
}
