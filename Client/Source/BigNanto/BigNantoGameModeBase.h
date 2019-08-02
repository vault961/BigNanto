// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BigNantoGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class BIGNANTO_API ABigNantoGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UUserWidget> LoginWidgetClass;

	UPROPERTY()
	UUserWidget* CurrentWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UBigNantoGameInstance* GameInstance;

public:
	ABigNantoGameModeBase();

	UFUNCTION(BlueprintCallable)
	void ChangeWidget(TSubclassOf<UUserWidget> NewWidegtClass);

};
