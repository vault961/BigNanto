// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RingOutExplosion.generated.h"

UCLASS()
class BIGNANTO_API ARingOutExplosion : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARingOutExplosion();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// ·çÆ® ¾À ÄÄÆ÷³ÍÆ®
	UPROPERTY(EditAnywhere)
	USceneComponent* RootSceneComponent;

	UPROPERTY(EditAnywhere)
	class ACenterViewPawn* CenterViewPawn;

	class UPaperFlipbookComponent* PaperFlipComponent;

	class UPaperFlipbook* RingOut_CPU;
	class UPaperFlipbook* RingOut_P1;
	class UPaperFlipbook* RingOut_P2;
	class UPaperFlipbook* RingOut_P3;
	class UPaperFlipbook* RingOut_P4;

	void SetExplosionDirection();
};
