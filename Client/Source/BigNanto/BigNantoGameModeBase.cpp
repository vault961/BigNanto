// Fill out your copyright notice in the Description page of Project Settings.


#include "BigNantoGameModeBase.h"
#include "BigNantoGameInstance.h"
#include "Blueprint/UserWidget.h"
#include "BigNantoPlayerController.h"
#include "PlayerCharacter.h"
#include "UObject/ConstructorHelpers.h"

ABigNantoGameModeBase::ABigNantoGameModeBase()
{
	//DefaultPawnClass = AWizardCharacter::StaticClass();
	//GameStateClass = AInGameStateBase::StaticClass();
	//HUDClass = AInGameHUD::StaticClass();
	PlayerControllerClass = ABigNantoPlayerController::StaticClass();
	static ConstructorHelpers::FClassFinder<UUserWidget> LoginWidget(TEXT("/Game/UMG/LoginUI"));
	if (LoginWidget.Succeeded())
		LoginWidgetClass = LoginWidget.Class;
}

void ABigNantoGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	GameInstance = Cast<UBigNantoGameInstance>(GetGameInstance());
	if (nullptr == GameInstance)
		return;

	ChangeWidget(LoginWidgetClass);
}

void ABigNantoGameModeBase::ChangeWidget(TSubclassOf<UUserWidget> NewWidegtClass)
{
	// ÇöÀç ÄÑÁ®ÀÖ´Â À§Á¬ÀÌ ÀÖ´Ù¸é ¹Ù²Ù±â À§ÇØ À§Á¬À» ²¨ÁÜ
	if (CurrentWidget != nullptr)
	{
		CurrentWidget->RemoveFromViewport();
		CurrentWidget = nullptr;
	}

	// ¹Ù²Ü À§Á¬À» ºäÆ÷Æ®¿¡ ±ò¾ÆÁÜ
	if (NewWidegtClass != nullptr)
	{
		CurrentWidget = CreateWidget(GetWorld(), NewWidegtClass);
		if (CurrentWidget != nullptr)
		{
			CurrentWidget->AddToViewport();
		}
	}
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
