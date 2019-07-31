// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM()
enum class EWeaponType : uint8
{
	ESword,
	EMagicWand,
};

UCLASS()
class BIGNANTO_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Weapon)
	class UBoxComponent* BaseCollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Weapon)
	class UStaticMeshComponent* WeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	int32 AttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	bool bIsActive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	class APlayerCharacter* WeaponOwner;

	UPROPERTY()
	EWeaponType WeaponType;
};
