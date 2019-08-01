// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// Sets default values
APlayerManager::APlayerManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APlayerManager::AddPlayers()
{
	// 월드에 있는 모든 액터들 가져오기
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerCharacter::StaticClass(), FoundActors);
	
	for (auto Actor : FoundActors)
	{
		APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(Actor);

		if (PlayerCharacter)
		{
			Players.Add(PlayerCharacter);
			++TotalPlayers;
			UE_LOG(LogTemp, Log, TEXT("'%s' in the house"), *PlayerCharacter->GetName());
		}
	}
	UE_LOG(LogTemp, Log, TEXT("Total Player : %d"), TotalPlayers);
}
