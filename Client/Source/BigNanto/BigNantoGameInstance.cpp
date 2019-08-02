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

//#pragma comment (lib, "Ws2_32.lib")

UBigNantoGameInstance::UBigNantoGameInstance()
{
	BufArraySize = 0;
	CurrentUserNum = 0;
	sumLen = 0;
	bIsConnected = false;
}

void UBigNantoGameInstance::Init()
{
	UE_LOG(LogTemp, Log, TEXT("GameInstance Initiate"));

	// 틱 돌아가기 시작
	TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UBigNantoGameInstance::Tick));
	//EnterGame("잉", 1 , "이잉", 3);
}

bool UBigNantoGameInstance::Tick(float DeltaTime)
{
	if (false == bIsConnected)
		return false;

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

	memcpy(BUF, &Type, 1);
	memcpy(BUF + TYPELEN + USERLEN, &size, 2);
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

		Len = *(wchar_t*)(targetArray + 2);
		if (BufArraySize >= Len) {
			Packet packet((PACKET_TYPE)(*targetArray), *(targetArray + 1), targetArray + 6, Len);
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


//return packet body size
int UBigNantoGameInstance::MakeLoginBuf(char * source, char cls, float Y, float Z, uint32 damage, char * name, int namelen) {
	memcpy(source, &cls, 1);
	memcpy(source + 1, &Y, 4);
	memcpy(source + 5, &Z, 4);
	memcpy(source + 9, &damage, 2);
	memcpy(source + 11, name, namelen);

	return namelen + 11;
}


void UBigNantoGameInstance::PacketProcess(Packet& packet) 
{
	FVector none;

	switch (packet.type) {
	case PACKET_TYPE::ENTER:
	{
		
		// 내 ID 받기
		MyID = packet.body[0];
		FVector RandomLocation = CharacterSpawner->GetRandomPointInVolume();

		char buf[20];
		int len = MakeLoginBuf(buf, 0, RandomLocation.Y, RandomLocation.Z, 0, "dsf", 3);
		// send class, y, z, damage, name to server TYPE PLAYERSPAWN
		// 서버에게 내 캐릭터 요청
		SendMessage(PACKET_TYPE::PLAYERSPAWN, buf, len);  ////////////////// WORK IN PROGRESS

		break;
	}
	case PACKET_TYPE::PLAYERSPAWN:
	{
		char CharacterClass = packet.body[0];
		float PosY = *(float*)(packet.body + sizeof(char));
		float PosZ = *(float*)(packet.body + sizeof(char) + sizeof(float));

		int DamagePercent = *(wchar_t*)(packet.body + sizeof(float) * 2 + sizeof(char));
		uint8* Name = packet.body + sizeof(float) * 2 + sizeof(char) + sizeof(wchar_t);
		int NameLen = packet.len - sizeof(float) * 2 + sizeof(char) + sizeof(wchar_t) - FRONTLEN;


		// 패킷 ID와 내 ID 비교
		// 내 캐릭터 스폰
		if (packet.userID == MyID) 
		{
			MyCharacter = CharacterSpawner->SpawnCharacter(CharacterClass, PosY, PosZ, DamagePercent,true);
			PlayerList[packet.userID] = MyCharacter;
			memcpy(MyCharacter->Name, Name, NameLen);
			MyCharacter->Name[NameLen] = '\0';


			if (nullptr == PlayerController)
				PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
			PlayerController->Possess(MyCharacter);

			CurrentUserNum++;
			UE_LOG(LogTemp, Warning, TEXT("myuser login"));
		}
		// 다른 캐릭터들 스폰
		else 
		{
			APlayerCharacter* OtherCha = CharacterSpawner->SpawnCharacter(CharacterClass, PosY, PosZ, DamagePercent,
				false);
			float movespeed = OtherCha->GetVelocity().Size();
			PlayerList[packet.userID] = OtherCha;
			memcpy(OtherCha->Name, Name, NameLen);
			OtherCha->Name[NameLen] = '\0';

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
		User->UpdateLocation(NewPosition);
		break;
	}
	case PACKET_TYPE::UPDATEDMG:
	{
		APlayerCharacter* User = PlayerList[packet.userID];
		User->DamagePercent = *(wchar_t*)(packet.body + 8);
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
		break;
	}
	}
}

void UBigNantoGameInstance::EnterGame(FString ServerIP, int32 ServerPort, FString UserName, uint8 ClassType)
{
	ConnectionSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("TCP"), false);
	UE_LOG(LogTemp, Log, TEXT("socket start"));
	if (!ConnectionSocket) {
		UE_LOG(LogTemp, Error, TEXT("Cannot create socket."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ServerIP = %s"), *ServerIP);
	UE_LOG(LogTemp, Log, TEXT("ServerPort = %d"), ServerPort);
	UE_LOG(LogTemp, Log, TEXT("UserName = %s"), *UserName);
	UE_LOG(LogTemp, Log, TEXT("ClassType = %d"), ClassType);
	
	//CharacterSpawner->SpawnCharacter(ClassType, 1, 1, 0, 1);

	TSharedRef<FInternetAddr> RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	FIPv4Address address;
	//FString IP = "172.18.33.156";			// 서버 아이피 (박진서)
	//FString IP = "172.18.33.158";			// 서버 아이피 (서영균)
	//FIPv4Address::Parse(IP, address);
	FIPv4Address::Parse(ServerIP, address);
	RemoteAddress->SetIp(address.Value);
	//RemoteAddress->SetPort(27015);			// 서버 포트
	RemoteAddress->SetPort(ServerPort);
	
	UE_LOG(LogTemp, Log, TEXT("흐으으으으으으으음"));

	if (ConnectionSocket->Connect(*RemoteAddress))
	{
		UE_LOG(LogTemp, Warning, TEXT("Connection done."));
		bIsConnected = true;
		return;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, FString::Printf(TEXT("Connection Failed")));
		bIsConnected = false;
		return;
	}

	// 게임 입장 패킷 전송
	//char empty[20] = "";
	//SendMessage(PACKET_TYPE::ENTER, "", 1);
}
