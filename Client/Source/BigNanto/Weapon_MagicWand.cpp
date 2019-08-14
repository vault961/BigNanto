// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon_MagicWand.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "UObject/ConstructorHelpers.h"

AWeapon_MagicWand::AWeapon_MagicWand()
{
	PrimaryActorTick.bCanEverTick = true;

	IncinerateHitBox = CreateDefaultSubobject<USphereComponent>(TEXT("IncinerateHitBox"));
	IncinerateHitBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	IncinerateHitBox->SetupAttachment(RootComponent);
	IncinerateHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleComponent"));
	ParticleComponent->SetupAttachment(IncinerateHitBox);
	ParticleComponent->bAutoActivate = false;
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleAsset(TEXT("/Game/StarterContent/Particles/P_Fire"));
	if (ParticleAsset.Succeeded())
	{
		ParticleComponent->SetTemplate(ParticleAsset.Object);
	}

	ChannelingTime = 0.f;
	HitBoxOnOffTime = 0.1f;
	IncinerateDamage = 15.f;
	bIsIncinerateOn = false;
	bIsHitboxOn = false;

	WeaponType = EWeaponType::EMagicWand;
}

void AWeapon_MagicWand::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Incinerate(DeltaTime);
}

void AWeapon_MagicWand::Incinerate(float DeltaTime)
{
	if (bIsIncinerateOn)
	{
		ChannelingTime += DeltaTime;
		if (ChannelingTime >= HitBoxOnOffTime)
		{
			bIsHitboxOn = !bIsHitboxOn;
			bIsHitboxOn ? IncinerateHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly)
				: IncinerateHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			ChannelingTime = 0.f;
		}
	}
	else
	{
		IncinerateHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ChannelingTime = 0.f;
		bIsHitboxOn = false;
	}
	ParticleComponent->Activate(!bIsIncinerateOn);
}
