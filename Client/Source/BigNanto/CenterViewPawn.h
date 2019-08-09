// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CenterViewPawn.generated.h"

UCLASS()
class BIGNANTO_API ACenterViewPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACenterViewPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	
	// 카메라 붐
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USpringArmComponent* CameraBoom;

	// 카메라 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UCameraComponent* CameraComponent;

	// 게임 인스턴스 레퍼런스
	class UBigNantoGameInstance* GameInstance;

	// 실시간 카메라 이동
	void UpdateCameraPosition();
};
