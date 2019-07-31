// Fill out your copyright notice in the Description page of Project Settings.


#include "BigNantoGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "PlayerCharacter.h"
#include "InGameHUD.h"
#include "InGameStateBase.h"
#include "WizardCharacter.h"

ABigNantoGameModeBase::ABigNantoGameModeBase()
{
	//DefaultPawnClass = AWizardCharacter::StaticClass();
	//GameStateClass = AInGameStateBase::StaticClass();
	//HUDClass = AInGameHUD::StaticClass();
}

//void ABigNantoGameModeBase::SpawnCharacters()
//{
//	TArray<AActor*> FoundActors;
//
//	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacterSpawner::StaticClass(), FoundActors);
//
//	for (auto Actor : FoundActors)
//	{
//		ACharacterSpawner* SpawnActor = Cast<ACharacterSpawner>(Actor);
//		if (SpawnActor)
//		{
//			//switch(TotalPlayers)
//			
//			GetWorld()->SpawnActor<APlayerCharacter>(Warrior.Get(), SpawnActor->GetActorLocation(), SpawnActor->GetActorRotation());
//		}
//	}
//}
