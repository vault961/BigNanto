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
	userNum = 0;
	sumLen = 0;
	// 워리어 클래스
	static ConstructorHelpers::FClassFinder<APawn> WarriorBP(TEXT("/Game/Blueprints/Warrior_BP"));
	if (WarriorBP.Class)
		Warrior = WarriorBP.Class;
}

void UBigNantoGameInstance::Init()
{
	UE_LOG(LogTemp, Log, TEXT("GameInstance Initiate"));

	FIPv4Endpoint RemoteAddressForConnection;
	const FString& YourChosenSocketName = "df";
	//uint8 IP4Nums[4];
	int32 ThePort = 27015;
	const int32 ReceiveBufferSize = 2 * 1024;
	int32 NewSize = 0;

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
	FString IP = "172.18.33.156";
	FIPv4Address::Parse(IP, address);
	RemoteAddress->SetIp(address.Value);
	RemoteAddress->SetPort(27015);

	if (ConnectionSocket->Connect(*RemoteAddress)) {
		UE_LOG(LogTemp, Warning, TEXT("Connection done."));
		//GetWorldTimerManager().SetTimer(SendMessTimerHandle, this, &ATCP::SendMessage, 2.f, false);
		//return ;
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Waiting for the server...")));
		return;
	}

	TArray<AActor*> FoundActors;
	//UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerController::StaticClass(), FoundActors);
	//PlayerController = Cast<APlayerController>(FoundActors[0]);

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacterSpawner::StaticClass(), FoundActors);
	SpawnActor = Cast<ACharacterSpawner>(FoundActors[0]);
	SpawnActor2 = Cast<ACharacterSpawner>(FoundActors[1]);

	char name[5] = "yeap";
	SendMessage(PACKET_TYPE::MYLOGIN, name, 10);
}

bool UBigNantoGameInstance::Tick(float DeltaTime)
{
	//UE_LOG(LogTemp, Warning, TEXT("GameInstance Ticking........."));

	PacketHandler();
	return true;
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
		Len = *(unsigned int*)(targetArray + 2);
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

void UBigNantoGameInstance::SendMessage(PACKET_TYPE Type, char * Body, uint32 size)
{
	//FString serialized = "I am the message from UE4.";
	//TCHAR* serializedChar = serialized.GetCharArray().GetData();
	//int32 size = FCString::Strlen(serializedChar) + 4;
	int32 sent = 0;
	char BUF[BUFLEN];

	memcpy(BUF, &Type, 1);
	memcpy(BUF + 2, &size, 4);
	memcpy(BUF + 6, Body, size - 6);

	bool successful = ConnectionSocket->Send((uint8*)BUF, size, sent);

	if (successful) {
		//UE_LOG(LogTemp, Warning, TEXT("Message Sent."));
	}

	return;
}

void UBigNantoGameInstance::PacketProcess(Packet& packet) 
{
	switch (packet.type) {
	case PACKET_TYPE::OTHERLOGIN:
		MyActor = GetWorld()->SpawnActorDeferred<APlayerCharacter>(Warrior.Get(), SpawnActor2->GetTransform());
		MyActor->IsMine = false;
		MyActor->FinishSpawning(SpawnActor2->GetTransform());
		PlayerList[packet.userID] = MyActor;
		UE_LOG(LogTemp, Warning, TEXT("other user login"));
		userNum++;
		break;
	case PACKET_TYPE::MYLOGIN:
		MyActor = GetWorld()->SpawnActorDeferred<APlayerCharacter>(Warrior.Get(), SpawnActor->GetTransform());
		MyActor->IsMine = true;
		MyActor->FinishSpawning(SpawnActor->GetTransform());
		PlayerList[packet.userID] = MyActor;
		MyPlayer = MyActor;
		if (nullptr == PlayerController)
		{
			PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		}
		PlayerController->Possess(MyActor);
		UE_LOG(LogTemp, Warning, TEXT("myuser login"));
		userNum++;
		break;
	case PACKET_TYPE::UPDATEPOS:
		NewPosition.Y = *(float*)packet.body;
		NewPosition.Z = *(float*)(packet.body + 4);
		//UE_LOG(LogTemp, Warning, TEXT("%d user : "), packet.userID);
		PlayerList[packet.userID]->UpdatePosition(NewPosition);
		break;
	case PACKET_TYPE::UPDATESTATUS:
		PlayerList[packet.userID]->UpdateStatus();
		break;
	}
}

void UBigNantoGameInstance::Shutdown()
{
	UE_LOG(LogTemp, Log, TEXT("GameInstance Shutdown..........."));
	FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
	ConnectionSocket->Close();
}