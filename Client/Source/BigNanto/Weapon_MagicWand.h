// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Weapon_MagicWand.generated.h"

/**
 * 
 */
UCLASS()
class BIGNANTO_API AWeapon_MagicWand : public AWeapon
{
	GENERATED_BODY()
public:
	AWeapon_MagicWand();
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USphereComponent* IncinerateHitBox;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UParticleSystemComponent* ParticleComponent;

	bool bIsIncinerateOn;
	bool bIsHitboxOn;
	float ChannelingTime;
	float HitBoxOnOffTime;
	float IncinerateDamage;

	void Incinerate(float DeltaTime);
};
