// Fill out your copyright notice in the Description page of Project Settings.


#include "BigNantoPlayerController.h"
#include "BigNantoGameInstance.h"


void ABigNantoPlayerController::BeginPlay()
{
	Super::BeginPlay();
	SetInputMode(FInputModeUIOnly());
	bShowMouseCursor = true;
	UE_LOG(LogTemp, Log, TEXT("내가 돌아왔다"));
}