// Fill out your copyright notice in the Description page of Project Settings.


#include "WizardCharacter.h"
#include "PlayerCharacterAnim.h"
#include "UObject/ConstructorHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Weapon_MagicWand.h"

AWizardCharacter::AWizardCharacter()
{
	CharacterClass = ECharacterClass::EWizard;
}

void AWizardCharacter::BeginPlay()
{
	Super::BeginPlay();
	MagicWand = Cast<AWeapon_MagicWand>(Weapon);
}

void AWizardCharacter::Incinerate()
{
	if (GetCurrentState() != ECharacterState::EIdle)
		return;

	if (AnimInstance)
	{
		AnimInstance->bIsChanneling = true;
		AnimInstance->PlayIncinerate();
		SetCurrentState(ECharacterState::EChanneling);
	}
}

void AWizardCharacter::StopIncinerate()
{
	if (AnimInstance)
	{
		AnimInstance->bIsChanneling = false;
	}
}
