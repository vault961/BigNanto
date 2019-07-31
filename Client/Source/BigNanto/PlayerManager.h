// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerManager.generated.h"

UCLASS()
class BIGNANTO_API APlayerManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlayerManager();

	// 플레이어들 정보를 담는 배열
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PlayerManager)
	TArray<class APlayerCharacter*> Players;
	
	// 워리어 블루프린트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PlayerManager)
	TSubclassOf<class APlayerCharacter> Warrior;

	// 총 플레이어 수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PlayerManager)
	uint8 TotalPlayers;

	UFUNCTION()
	void AddPlayers();
};

