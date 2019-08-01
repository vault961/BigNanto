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
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<class APlayerCharacter*> PlayerArray;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class APlayerCharacter* MyCharacter;

	// �� �÷��̾� ��
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint8 TotalPlayers;

	UFUNCTION()
	void AddPlayers();
};

