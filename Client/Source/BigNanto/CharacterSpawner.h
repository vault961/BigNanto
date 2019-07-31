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

	// 스폰 박스
	UPROPERTY(VisibleInstanceOnly, Category = Spawning)
	class UBoxComponent* WhereToSpawn;

	// 전사
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Spawning)
	TSubclassOf<class APlayerCharacter> Warrior;

	// 마법사
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Spawning)
	TSubclassOf<class APlayerCharacter> Wizard;

	// Get 랜덤 위치
	UFUNCTION(BlueprintPure, Category = Spawning)
	FVector GetRandomPointInVolume();

	// 캐릭터 스폰 함수 (인자: 캐릭터 번호 (1 = 워리어, 2 = 위자드), IsMine인지 아닌지)
	APlayerCharacter* SpawnCharacter(int CharacterNum, bool bIsMine);
};
