// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CharacterSpawner.generated.h"

UCLASS()
class BIGNANTO_API ACharacterSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	ACharacterSpawner();
	virtual void BeginPlay() override;

	// ���� �ڽ�
	UPROPERTY(VisibleInstanceOnly, Category = Spawning)
	class UBoxComponent* WhereToSpawn;

	// ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Spawning)
	TSubclassOf<class APlayerCharacter> Warrior;

	// ������
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Spawning)
	TSubclassOf<class APlayerCharacter> Wizard;

	// Get ���� ��ġ
	UFUNCTION(BlueprintPure, Category = Spawning)
	FVector GetRandomPointInVolume();

	// ĳ���� ���� �Լ� (����: ĳ���� ��ȣ (1 = ������, 2 = ���ڵ�), IsMine���� �ƴ���)
	APlayerCharacter* SpawnCharacter(int CharacterNum, bool bIsMine);
};
