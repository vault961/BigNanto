﻿// Fill out your copyright notice in the Description page of Project Settings.


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
#include "BigNantoPlayerController.h"
#include "BigNantoGameModeBase.h"
#include "CenterViewPawn.h"

UBigNantoGameInstance::UBigNantoGameInstance()
{
	NewPosition.X = 0;
	BufArraySize = 0;
	CurrentUserNum = 0;
	sumLen = 0;
	MyName = "Unknown";
	MyClassType = 1;
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

void UBigNantoGameInstance::SendMessage(PACKET_TYPE Type, char * Body, uint32 BodySize)
{
	
	uint32 size = BodySize + FRONTLEN;
	int32 sent = 0;

	uint8 BUF[BUFLEN];

	FMemory::Memcpy(BUF + LENLEN + USERLEN, &Type, TYPELEN);
	FMemory::Memcpy(BUF, &size, LENLEN);
	FMemory::Memcpy(BUF + FRONTLEN, Body, BodySize);

	do {
		//if (Type == PACKET_TYPE::UPDATESTATE) UE_LOG(LogTemp, Error, TEXT("%d %d"), size, *(BUF+FRONTLEN));

		bool successful = ConnectionSocket->Send(BUF, size, sent);

		if (!successful) {
			UE_LOG(LogTemp, Error, TEXT("Message can't send!!!!!!!!"));
		}
		size -= sent;

	} while (size > 0);


	return;
}

template <typename T>
void UBigNantoGameInstance::DataAddCopy(char * source, T* get, int size, int& sum) {
	FMemory::Memcpy(source + sum, get, size);
	sum += size;
}
template <typename T>
void UBigNantoGameInstance::DataAddGet(T* source, char* get, int size, int& sum) {
	FMemory::Memcpy(source, get + sum, size);
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
			FMemory::Memcpy(BufArray + BufArraySize, ReadData, ReadBytes);
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
	switch (packet.type) {
	case PACKET_TYPE::NAMECHECK:
	{
		// 중복된 이름없을 때
		switch (packet.body[0]) {
		case (char)ERRORCODE::NOTERROR:
		{
			//GameModeBase->RemoveAllWidget();

			GameModeBase->ChangeWidget(GameModeBase->InGameWidgetClass);
			PlayerController->OnGameMode();

			// 내 ID 받기
			MyID = packet.userID;

			// 랜덤한 위치에 소환하기 위해 랜덤벡터 생성
			FVector RandomLocation = CharacterSpawner->GetRandomPointInVolume();

			// 직업타임, Y좌표, Z좌표, 데미지 퍼센트, 이름 PlayerSpawn 타입 패킷으로 전송
			char buf[100]{ 0 };

			int len = MakeLoginBuf(buf, MyClassType, RandomLocation.Y, RandomLocation.Z, 0, 0, TCHAR_TO_UTF8(*MyName), MyName.Len() + 1);
			//UE_LOG(LogTemp, Warning, TEXT("%s"), MyName.GetCharArray().GetData());
			
			// 서버에게 내 캐릭터 요청
			SendMessage(PACKET_TYPE::PLAYERSPAWN, buf, len);
		}
			break;
		case (char)ERRORCODE::TOOLONG:
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT(" 최대 10자 입니다."));
			break;
		case (char)ERRORCODE::ALREADYNAME:
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT(" 중복된 이름입니다."));
			break;
		case (char)ERRORCODE::NOTENGLISH:
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT(" 영어로만 입력해주세요"));
			break;
		case (char)ERRORCODE::TOOSHORT:
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT(" 1자 이상으로 입력해주세요"));
			break;
		}
		
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
		uint32 KillCount;

		uint8 PlayerName[11]{ 0 };

		DataAddGet(&CharacterClass, source, sizeof(char), sum);
		DataAddGet(&PosY, source, sizeof(float), sum);
		DataAddGet(&PosZ, source, sizeof(float), sum);
		DataAddGet(&DamagePercent, source, sizeof(float), sum);
		DataAddGet(&KillCount, source, sizeof(uint32), sum);

		int NameLen = packet.len - sum - FRONTLEN;
		DataAddGet(PlayerName, source, NameLen, sum);

		APlayerCharacter* Character;

		// 패킷 ID와 내 ID 비교
		// 내 캐릭터 스폰
		if (packet.userID == MyID)
		{
			Character = CharacterSpawner->SpawnCharacter(CharacterClass, PosY, PosZ, DamagePercent, KillCount, true);
			if (nullptr == PlayerController)
				PlayerController = Cast<ABigNantoPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

			PlayerController->Possess(Character);
			//PlayerController->SetViewTargetWithBlend(CenterViewPawn);
		}
		// 다른 캐릭터들 스폰
		else
		{
			Character = CharacterSpawner->SpawnCharacter(CharacterClass, PosY, PosZ, DamagePercent, KillCount, false);
		}

		if (nullptr == Character)
		{
			UE_LOG(LogTemp, Error, TEXT("내 캐릭터를 정상적으로 스폰할 수 없습니다"));
			return;
		}

		if(!PlayerList.Contains(packet.userID))
			PlayerList.Add(packet.userID, Character);

		Character->PlayerName = FString(UTF8_TO_TCHAR(PlayerName));
		Character->PlayerID = packet.userID;

		break;
	}
	case PACKET_TYPE::UPDATELOCATION:
	{
		APlayerCharacter* User = nullptr;
		if (PlayerList.Contains(packet.userID))
			User = PlayerList[packet.userID];
		if (nullptr == User)
			return;
		FVector fv(0, *(float*)packet.body, *(float*)(packet.body + 4));
		User->UpdateLocation(fv, (uint8)*(char*)(packet.body + 8));
		//UE_LOG(LogTemp, Warning, TEXT("userid:%d y:%f z:%f"),packet.userID, fv.Y, fv.Z);

		break;
	}
	case PACKET_TYPE::UPDATEDMG:
	{
		APlayerCharacter* User = nullptr;
		if (PlayerList.Contains(packet.userID))
			User = PlayerList[packet.userID];
		if (nullptr == User)
			return;
		User->DamagePercent = *(float*)packet.body;

		break;
	}
	case PACKET_TYPE::UPDATESTATE:
	{
		if (packet.userID != MyID) {
			APlayerCharacter* User = nullptr;
			if (PlayerList.Contains(packet.userID))
				User = PlayerList[packet.userID];
			if (nullptr == User)
				return;
			switch (packet.body[0]) {
			case (char)ECharacterAction::EA_Attack:
				User->Attack();
				break;
			case (char)ECharacterAction::EA_Jump:
				User->DoJump();
				break;
			case (char)ECharacterAction::EA_StopAttack:
				User->StopAttack();
				break;
			case (char)ECharacterAction::EA_Die:
			{
				User->PlayRingOutEffect();
				User->Destroy();
				if(PlayerList.Contains(packet.userID))
					PlayerList.Remove(packet.userID);
			}
				break;
			case (char)ECharacterAction::EA_Move:
				User->AnimInstance->bIsMoving = true;
				break;
			case (char)ECharacterAction::EA_StopMove:
				User->AnimInstance->bIsMoving = false;
				break;
			case (char)ECharacterAction::EA_SpecialAbility:
				User->SpecialAbility();
				break;
			case (char)ECharacterAction::EA_StopSpecialAbility:
				User->StopSpecialAbility();
				break;
			case (char)ECharacterAction::EA_IgnorePlatform:
				User->IgnorePlatform(true);
				break;
			case (char)ECharacterAction::EA_BlockPlatform:
				User->IgnorePlatform(false);
				break;
			}
		}
		break;
	}
	case PACKET_TYPE::LOGOUT:
	{
		// 플레이어 리스트에 해당 캐릭터가 남아있는지 체크 
		APlayerCharacter* User = nullptr;
		if (PlayerList.Contains(packet.userID))
			User = PlayerList[packet.userID];
		
		if (nullptr == User)
			return;

		User->Destroy();
		PlayerList.Remove(packet.userID);

		break;
	}
	case PACKET_TYPE::KILL:
	{
		uint32 KillerID = *(uint32*)(packet.body);
		APlayerCharacter *Killer = nullptr;
		if (PlayerList.Contains(KillerID))
			Killer = PlayerList[KillerID];
		if (nullptr != Killer) {
			Killer->KillCount++;
		}

		break;
	}
	}
}

int UBigNantoGameInstance::MakeLoginBuf(char * source, char cls, float Y, float Z, float damage, uint32 KillCount, char * name, int namelen) {
	int sum = 0;
	DataAddCopy(source, &cls, sizeof(char), sum);
	DataAddCopy(source, &Y, sizeof(float), sum);
	DataAddCopy(source, &Z, sizeof(float), sum);
	DataAddCopy(source, &damage, sizeof(float), sum);
	DataAddCopy(source, &KillCount, sizeof(uint32), sum);
	DataAddCopy(source, name, namelen, sum);
	return sum;
}


void UBigNantoGameInstance::NameCheck(FString UserName, uint8 ClassType)
{
	MyName = UserName + '\0';
	MyClassType = ClassType;

	//if (MyName.Len() > 10)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("이름은 10자를 넘을 수 없습니다"));
	//	return;
	//}
	
	//for (auto it : MyName.GetCharArray()) {
	//	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("") +it);
	//	if(!('0' <= it && it <= 'z'))
	//	{

	//		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, it + TEXT("는 적절한 이름이 아닙니다"));
	//		return;
	//	}
	//}

	if (nullptr == CharacterSpawner)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("캐릭터 스포너가 없습니다!!!!"));
	}

	SendMessage(PACKET_TYPE::NAMECHECK, TCHAR_TO_UTF8(*UserName), UserName.Len());
}

bool UBigNantoGameInstance::EnterGame(FString ServerIP, int32 ServerPort)
{
	ConnectionSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("TCP"), false);
	if (!ConnectionSocket) {
		UE_LOG(LogTemp, Error, TEXT("Cannot create socket."));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("ServerIP = %s"), *ServerIP);
	UE_LOG(LogTemp, Log, TEXT("ServerPort = %d"), ServerPort);

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

		return true;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, FString::Printf(TEXT("Connection Failed")));
		return false;
	}
}
