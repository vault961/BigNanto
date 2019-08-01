// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterSpawner.h"
#include "Components/BoxComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "PlayerCharacter.h"
#include "BigNantoGameInstance.h"

// Sets default values
ACharacterSpawner::ACharacterSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	WhereToSpawn = CreateDefaultSubobject<UBoxComponent>(TEXT("WhereToSpawn"));

	// 워리어 클래스
	static ConstructorHelpers::FClassFinder<APawn> WarriorBP(TEXT("/Game/Blueprints/Warrior_BP"));
	if (WarriorBP.Class)
		Warrior = WarriorBP.Class;

	// 위자드 클래스
	static ConstructorHelpers::FClassFinder<APawn> WizardBP(TEXT("/Game/Blueprints/Wizard_BP"));
	if (WizardBP.Class)
		Wizard = WizardBP.Class;
}

void ACharacterSpawner::BeginPlay()
{
	Super::BeginPlay();

	UBigNantoGameInstance* GameInstance = Cast<UBigNantoGameInstance>(GetGameInstance());
	if (!GameInstance)
		return;
	GameInstance->CharacterSpawner = this;
}

FVector ACharacterSpawner::GetRandomPointInVolume()
{
	FVector RandomLocation;
	float MinY, MinZ;
	float MaxY, MaxZ;

	FVector Origin;
	FVector BoxExtent;

	// get the spawnVolume's origin and box extent
	Origin = WhereToSpawn->Bounds.Origin;
	BoxExtent = WhereToSpawn->Bounds.BoxExtent;

	// Calculate the minimum X, Y, and Z
	MinY = Origin.Y - BoxExtent.Y / 2.f;
	MinZ = Origin.Z - BoxExtent.Z / 2.f;

	// Calculate the maximum X, Y, and Z
	MaxY = Origin.Y + BoxExtent.Y / 2.f;
	MaxZ = Origin.Z + BoxExtent.Z / 2.f;

	// The random spawn location will fall between the min and max X, Y, and Z
	RandomLocation.X = 0.f;
	RandomLocation.Y = FMath::FRandRange(MinY, MaxY);
	RandomLocation.Z = FMath::FRandRange(MinZ, MaxZ);

	// Return the random spawn location
	return RandomLocation;
}

APlayerCharacter* ACharacterSpawner::SpawnCharacter(char CharacterNum, float PosY, float PosZ, int Damage, bool bIsMine)
{
	// 월드 체크
	UWorld* const World = GetWorld();
	if (World)
	{
		//FTransform SpawnTransform(GetRandomPointInVolume());
		
		APlayerCharacter* Character;

		switch (CharacterNum)
		{
		case 1:
			Character = World->SpawnActorDeferred<APlayerCharacter>(Warrior.Get(), SpawnTransform);
			break;
		case 2:
			Character = World->SpawnActorDeferred<APlayerCharacter>(Wizard.Get(), SpawnTransform);
			break;
		default:
			UE_LOG(LogTemp, Error, TEXT("There is no character number %d"), CharacterNum);
			return nullptr;
		}

		if (Character == nullptr)
			return nullptr;

		Character->IsMine = bIsMine;
		Character->FinishSpawning(SpawnTransform);
		return Character;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("THERE IS NO WORLD!!!!!!!"));
		return nullptr;
	}

}
