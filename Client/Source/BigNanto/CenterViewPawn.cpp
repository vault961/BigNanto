// Fill out your copyright notice in the Description page of Project Settings.


#include "CenterViewPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "BigNantoGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "BigNantoGameInstance.h"
#include "PlayerCharacter.h"

// Sets default values
ACenterViewPawn::ACenterViewPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 2000.f;											// 카메라 거리
	CameraBoom->SocketOffset = FVector(0.f, 0.f, 75.f);								// 카메라 오프셋 위치 (높이 조정)
	CameraBoom->RelativeRotation = FRotator(0.f, 180.f, 0.f);						// 카메라 회전각도
	CameraBoom->bDoCollisionTest = false;											// 충돌체크 안함
	CameraBoom->bEnableCameraLag = true;											// 카메라 레그

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("SideViewCamera"));
	CameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
}

// Called when the game starts or when spawned
void ACenterViewPawn::BeginPlay()
{
	Super::BeginPlay();

	GameInstance = Cast<UBigNantoGameInstance>(GetGameInstance());
	if (nullptr != GameInstance)
	{
		GameInstance->CenterViewPawn = this;
	}
}

void ACenterViewPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);;
	if (PlayerController)
	{
		PlayerController->SetViewTarget(this);
	}

	UpdateCameraPosition();
}

void ACenterViewPawn::UpdateCameraPosition()
{
	auto PlayerListNum = GameInstance->PlayerList.Num();
	// 플레이어 인원이 0이하라면 업데이트하지 않고 종료
	if (PlayerListNum <= 0)
		return;

	float TargetPosY = 0.f;
	float TargetPosZ = 0.f;

	float LeftMost = -10000.f;
	float RightMost = 10000.f;

	// 이녀석들은 안쓰는중
	float TopMost = -10000.f;
	float BottomMost = 10000.f;

	float MinArmLength = 1000.f;
	float MaxArmLength = 2000.f;

	// 플레이어 리스트 돌면서 플레이어들의 중간 지점을 계산
	for (auto it : GameInstance->PlayerList)
	{
		const float PlayerPosY = it.Value->GetActorLocation().Y;
		const float PlayerPosZ = it.Value->GetActorLocation().Z;

		TargetPosY += PlayerPosY;
		TargetPosZ += PlayerPosZ;

		// 가장 좌측, 우측에 있는 플레이어 포지션 구하기
		// 좌측으로 갈 수록 Y포지션이 커진다요
		// LeftMost가 가장 큰값이다요
		if (PlayerPosY > LeftMost) LeftMost = PlayerPosY;
		if (PlayerPosY < RightMost) RightMost = PlayerPosY;

		//if (PlayerPosZ > TopMost) TopMost = PlayerPosZ;
	}

	FVector TargetPos = GetActorLocation();
	TargetPos.Y = TargetPosY / PlayerListNum;
	TargetPos.Z = TargetPosZ / PlayerListNum;

	if (PlayerListNum  == 1)
	{
		CameraBoom->TargetArmLength = MinArmLength;
	}
	else
	{
		CameraBoom->TargetArmLength = FMath::Clamp(MinArmLength + (LeftMost - RightMost), MinArmLength, MaxArmLength);
	}

	SetActorLocation(TargetPos);
}

