// Fill out your copyright notice in the Description page of Project Settings.


#include "BigNantoGameModeBase.h"
#include "BigNantoGameInstance.h"
#include "Blueprint/UserWidget.h"
#include "BigNantoPlayerController.h"
#include "PlayerCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "CenterViewPawn.h"

ABigNantoGameModeBase::ABigNantoGameModeBase()
{
	DefaultPawnClass = ACenterViewPawn::StaticClass();
	PlayerControllerClass = ABigNantoPlayerController::StaticClass();

	static ConstructorHelpers::FClassFinder<UUserWidget> LoginWidget(TEXT("/Game/UMG/LoginUI"));
	if (LoginWidget.Succeeded())
		LoginWidgetClass = LoginWidget.Class;

	static ConstructorHelpers::FClassFinder<UUserWidget> CharacterMakeWidget(TEXT("/Game/UMG/CharacterMakeUI"));
	if (CharacterMakeWidget.Succeeded())
		CharacterMakeWidgetClass = CharacterMakeWidget.Class;
}

void ABigNantoGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	GameInstance = Cast<UBigNantoGameInstance>(GetGameInstance());
	if (nullptr == GameInstance)
		return;

	GameInstance->GameModeBase = this;

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

void ABigNantoGameModeBase::RemoveAllWidget()
{
	if (CurrentWidget != nullptr)
	{
		CurrentWidget->RemoveFromViewport();
		CurrentWidget = nullptr;
		GameInstance->PlayerController->SetInputMode(FInputModeGameOnly());
		GameInstance->PlayerController->bShowMouseCursor = false;
	}
}
