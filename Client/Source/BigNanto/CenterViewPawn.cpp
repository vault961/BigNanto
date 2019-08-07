// Fill out your copyright notice in the Description page of Project Settings.


#include "CenterViewPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "BigNantoGameInstance.h"

// Sets default values
ACenterViewPawn::ACenterViewPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 2000.f;											// 카메라 거리
	CameraBoom->SocketOffset = FVector(0.f, 0.f, 75.f);								// 카메라 오프셋 위치 (높이 조정)
	CameraBoom->RelativeRotation = FRotator(0.f, 180.f, 0.f);						// 카메라 회전각도
	CameraBoom->bDoCollisionTest = false;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("SideViewCamera"));
	CameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
}

// Called when the game starts or when spawned
void ACenterViewPawn::BeginPlay()
{
	Super::BeginPlay();
	
	UBigNantoGameInstance* GameInstance = Cast<UBigNantoGameInstance>(GetGameInstance());
	if (nullptr != GameInstance)
	{
		GameInstance->CenterViewPawn = this;
	}
}

