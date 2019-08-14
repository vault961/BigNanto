// Fill out your copyright notice in the Description page of Project Settings.


#include "RingOutExplosion.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "UObject/ConstructorHelpers.h"
#include "BigNantoGameInstance.h"
#include "CenterViewPawn.h"

// Sets default values
ARingOutExplosion::ARingOutExplosion()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RingOutExplosion"));
	SetRootComponent(RootSceneComponent);

	PaperFlipComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("PaperFlipComponents"));
	PaperFlipComponent->SetupAttachment(RootComponent);
	PaperFlipComponent->RelativeLocation = FVector(0.f, 0.f, 1400.f);
	PaperFlipComponent->RelativeRotation = FRotator(0.f, 90.f, 0.f);
	PaperFlipComponent->RelativeScale3D = FVector(3.f);
	PaperFlipComponent->SetLooping(false);

	RingOut_CPU = ConstructorHelpers::FObjectFinder<UPaperFlipbook>
		(TEXT("/Game/Particles/RingOut/RingOut_CPU")).Object;
	RingOut_P1 = ConstructorHelpers::FObjectFinder<UPaperFlipbook>
		(TEXT("/Game/Particles/RingOut/RingOut_P1")).Object;
	RingOut_P2 = ConstructorHelpers::FObjectFinder<UPaperFlipbook>
		(TEXT("/Game/Particles/RingOut/RingOut_P2")).Object;
	RingOut_P3 = ConstructorHelpers::FObjectFinder<UPaperFlipbook>
		(TEXT("/Game/Particles/RingOut/RingOut_P3")).Object;
	RingOut_P4 = ConstructorHelpers::FObjectFinder<UPaperFlipbook>
		(TEXT("/Game/Particles/RingOut/RingOut_P4")).Object;
}

// Called when the game starts or when spawned
void ARingOutExplosion::BeginPlay()
{
	Super::BeginPlay();

	uint8 RandomNum = FMath::RandRange(1, 5);
	UE_LOG(LogTemp, Log, TEXT("%d"), RandomNum);
	switch (RandomNum)
	{
	case 1:
		PaperFlipComponent->SetFlipbook(RingOut_CPU);
		break;
	case 2:
		PaperFlipComponent->SetFlipbook(RingOut_P1);
		break;
	case 3:
		PaperFlipComponent->SetFlipbook(RingOut_P2);
		break;
	case 4:
		PaperFlipComponent->SetFlipbook(RingOut_P3);
		break;
	case 5:
		PaperFlipComponent->SetFlipbook(RingOut_P4);
		break;
	}


	SetExplosionDirection();
}

// Called every frame
void ARingOutExplosion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (false == PaperFlipComponent->IsPlaying())
		Destroy();
}

void ARingOutExplosion::SetExplosionDirection()
{
	UBigNantoGameInstance* GameInstance = Cast<UBigNantoGameInstance>(GetGameInstance());
	CenterViewPawn = GameInstance->CenterViewPawn;

	FVector Target = CenterViewPawn->GetActorLocation();
	FVector Start = GetActorLocation();
	FVector dir = (Target - Start).GetSafeNormal();

	float Dot = FVector::DotProduct(FVector::UpVector, dir);
	float AcosAngle = FMath::Acos(Dot);
	float Angle = FMath::RadiansToDegrees(AcosAngle);

	FVector Cross = FVector::CrossProduct(FVector::ForwardVector, dir);
	if (Cross.Z < 0) Angle = -Angle;

	SetActorRotation(FRotator(0.f, 0.f, Angle));
}

