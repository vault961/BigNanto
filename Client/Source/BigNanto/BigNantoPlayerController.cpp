// Fill out your copyright notice in the Description page of Project Settings.


#include "BigNantoPlayerController.h"
#include "BigNantoGameInstance.h"


void ABigNantoPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bAutoManageActiveCameraTarget = false;

	OnGameUI();

	UBigNantoGameInstance* GameInstance = Cast<UBigNantoGameInstance>(GetGameInstance());
	if (nullptr != GameInstance)
	{
		GameInstance->PlayerController = this;
	}
}

void ABigNantoPlayerController::OnGameUI()
{
	SetInputMode(FInputModeUIOnly());
	bShowMouseCursor = true;
}

void ABigNantoPlayerController::OffGameUI()
{
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
}
