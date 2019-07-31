// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter.h"
#include "WizardCharacter.generated.h"

/**
 * 
 */
UCLASS()
class BIGNANTO_API AWizardCharacter : public APlayerCharacter
{
	GENERATED_BODY()
public:
	AWizardCharacter();
	virtual void BeginPlay() override;
	class UParticleSystemComponent* IncinerateParticle;
	class AWeapon_MagicWand* MagicWand;
	virtual void SpecialAbility() override { Incinerate(); }
	virtual void StopSpecialAbility() override { StopIncinerate(); }

	void Incinerate();
	void StopIncinerate();
};
