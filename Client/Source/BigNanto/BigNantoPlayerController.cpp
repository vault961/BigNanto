// Fill out your copyright notice in the Description page of Project Settings.


#include "BigNantoPlayerController.h"
#include "BigNantoGameInstance.h"


void ABigNantoPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetInputMode(FInputModeUIOnly());
	bShowMouseCursor = true;

	UBigNantoGameInstance* GameInstance = Cast<UBigNantoGameInstance>(GetGameInstance());
	if (nullptr != GameInstance)
	{
		//GameInstance->PlayerController = this;
	}
}