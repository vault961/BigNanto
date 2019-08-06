// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PlayerCharacter.h"

// Sets default values
AWeapon::AWeapon()
{
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	BaseCollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BaseBoxComponent"));
	
	RootComponent = WeaponMesh;
	BaseCollisionComponent->SetupAttachment(WeaponMesh);

	WeaponMesh->SetCollisionProfileName(TEXT("NoCollision"));
	BaseCollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	AttackDamage = 10.f;
	bIsActive = false;

	WeaponType = EWeaponType::ESword;
}

