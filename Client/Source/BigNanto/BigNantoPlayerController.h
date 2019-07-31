// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BigNantoPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BIGNANTO_API ABigNantoPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

};