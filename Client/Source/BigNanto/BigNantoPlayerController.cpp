// Fill out your copyright notice in the Description page of Project Settings.


#include "BigNantoPlayerController.h"
#include "BigNantoGameInstance.h"


void ABigNantoPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bAutoManageActiveCameraTarget = false;

	OnUIMode();

	UBigNantoGameInstance* GameInstance = Cast<UBigNantoGameInstance>(GetGameInstance());
	if (nullptr != GameInstance)
	{
		GameInstance->PlayerController = this;
	}
}

void ABigNantoPlayerController::OnUIMode()
{
	SetInputMode(FInputModeUIOnly());
	bShowMouseCursor = true;
}

void ABigNantoPlayerController::OnGameMode()
{
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
}
