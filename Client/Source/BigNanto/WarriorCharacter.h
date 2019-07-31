// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter.h"
#include "WarriorCharacter.generated.h"

/**
 * 
 */
UCLASS()
class BIGNANTO_API AWarriorCharacter : public APlayerCharacter
{
	GENERATED_BODY()
public:
	AWarriorCharacter();

	virtual void SpecialAbility() override;
	virtual void StopSpecialAbility() override;

	// ���
	void Defend();
	void StopDefend();
};
