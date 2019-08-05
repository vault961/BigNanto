// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerCharacterAnim.generated.h"

UCLASS()
class BIGNANTO_API UPlayerCharacterAnim : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UPlayerCharacterAnim();
	~UPlayerCharacterAnim();
	
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float MoveSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsFalling;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsAttacking;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsDefending;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsChanneling;

	UPROPERTY()
	class UAnimMontage* HitMontage;

	UPROPERTY()
	class UAnimMontage* AttackMontage;
	
	UPROPERTY()
	class UAnimMontage* IncinerateMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class APlayerCharacter* PlayerCharacter;

	void PlayGetHit();
	void PlayDefendHit();
	void PlayAttack();
	void PlayIncinerate();
};
