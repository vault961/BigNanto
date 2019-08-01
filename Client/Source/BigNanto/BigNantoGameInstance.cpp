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

//#pragma comment (lib, "Ws2_32.lib")

UBigNantoGameInstance::UBigNantoGameInstance()
{
	BufArraySize = 0;
	CurrentUserNum = 0;
	sumLen = 0;
}

void UBigNantoGameInstance::Init()
{
	UE_LOG(LogTemp, Log, TEXT("GameInstance Initiate"));

	// 틱 돌아가기 시작
	TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UBigNantoGameInstance::Tick));

	ConnectionSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("TCP"), false);
	UE_LOG(LogTemp, Log, TEXT("socket start"));
	if (!ConnectionSocket) {
		UE_LOG(LogTemp, Error, TEXT("Cannot create socket."));
		return;
	}

	TSharedRef<FInternetAddr> RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	FIPv4Address address;
	//FString IP = "172.18.33.156";			// 서버 아이피 (박진서)
	FString IP = "172.18.33.158";			// 서버 아이피 (서영균)
	FIPv4Address::Parse(IP, address);
	RemoteAddress->SetIp(address.Value);
	RemoteAddress->SetPort(27015);			// 서버 포트

	if (ConnectionSocket->Connect(*RemoteAddress)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Connection done."));
	}
	else 
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Waiting for the server...")));
		return;
	}

	// 게임 입장 패킷 전송
	//char empty[1] = "";
	//SendMessage(PACKET_TYPE::ENTER, "", 1);
}

bool UBigNantoGameInstance::Tick(float DeltaTime)
{
	if (!MyID)
		return false;

	PacketHandler();
	return true;
}

void UBigNantoGameInstance::Shutdown()
{
	UE_LOG(LogTemp, Log, TEXT("GameInstance Shutdown..........."));
	FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
	ConnectionSocket->Close();
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

void UBigNantoGameInstance::SendMessage(PACKET_TYPE Type, char * Body, wchar_t BodySize)
{
	uint32 size = (uint32)BodySize + 6;
	int32 sent = 0;

	char BUF[BUFLEN];

	memcpy(BUF, &Type, 1);
	memcpy(BUF + TYPELEN + USERLEN, &size, 2);
	memcpy(BUF + FRONTLEN, Body, BodySize);

	bool successful = ConnectionSocket->Send((uint8*)BUF, size, sent);

	if (!successful) {
		UE_LOG(LogTemp, Error, TEXT("Message can't send!!!!!!!!"));
	}

	return;
}

void UBigNantoGameInstance::PacketProcess(Packet& packet) 
{
	switch (packet.type) {
	case PACKET_TYPE::ENTER:
	{
		// 내 ID 받기
		MyID = packet.body[0];
		FVector RandomLocation = CharacterSpawner->GetRandomPointInVolume();
		// send class, y, z, damage, name to server TYPE PLAYERSPAWN
		// 서버에게 내 캐릭터 요청
		//SendMessage(PACKET_TYPE::PLAYERSPAWN,     ,     );  ////////////////// WORK IN PROGRESS

		break;
	}
	case PACKET_TYPE::PLAYERSPAWN:
	{
		char CharacterClass = packet.body[0];
		float PosY = *(float*)(packet.body + sizeof(char));
		float PosZ = *(float*)(packet.body + sizeof(char) + sizeof(float));
		int DamagePercent = *(wchar_t*)(packet.body + sizeof(float) * 2 + sizeof(char));
		uint8* Name = packet.body + sizeof(float) * 2 + sizeof(char) + sizeof(wchar_t);
		int NameLen = packet.len - sizeof(float) * 2 + sizeof(char) + sizeof(wchar_t) - FRONTLEN - TIMESTAMPLEN;

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
			PlayerList[packet.userID] = OtherCha;
			memcpy(OtherCha->Name, Name, NameLen);
			OtherCha->Name[NameLen] = '\0';

			CurrentUserNum++;
			UE_LOG(LogTemp, Warning, TEXT("other user login"));
		}
		break;
	}
	case PACKET_TYPE::UPDATEDATA:
	{
		APlayerCharacter* User = PlayerList[packet.userID];
		NewPosition.Y = *(float*)packet.body;
		NewPosition.Z = *(float*)(packet.body + 4);
		User->DamagePercent = *(wchar_t*)(packet.body + 8);
		//UE_LOG(LogTemp, Warning, TEXT("%d user : "), packet.userID);
		User->UpdatePosition(NewPosition);
		break;
	}
	}
}
