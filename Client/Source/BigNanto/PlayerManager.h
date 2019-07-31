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

	// �÷��̾�� ������ ��� �迭
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PlayerManager)
	TArray<class APlayerCharacter*> Players;
	
	// ������ �������Ʈ
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PlayerManager)
	TSubclassOf<class APlayerCharacter> Warrior;

	// �� �÷��̾� ��
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PlayerManager)
	uint8 TotalPlayers;

	UFUNCTION()
	void AddPlayers();
};

