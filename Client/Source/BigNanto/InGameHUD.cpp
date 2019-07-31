// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameHUD.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "PlayerManager.h"

AInGameHUD::AInGameHUD()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> HUDWidetObject(TEXT("/Game/UMG/InGameHUD"));
	if (HUDWidetObject.Succeeded())
		HUDWidgetClass = HUDWidetObject.Class;
}

void AInGameHUD::BeginPlay()
{
	Super::BeginPlay();

	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UUserWidget>(this->GetOwningPlayerController(), this->HUDWidgetClass);
		HUDWidget->AddToViewport();
	}
}
