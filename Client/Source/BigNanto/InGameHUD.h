// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "InGameHUD.generated.h"

/**
 * 
 */
UCLASS()
class BIGNANTO_API AInGameHUD : public AHUD
{
	GENERATED_BODY()
public:
	AInGameHUD();
	virtual void BeginPlay() override;

	UPROPERTY()
	class UClass* HUDWidgetClass;
	UPROPERTY()
	class UUserWidget* HUDWidget;
};
