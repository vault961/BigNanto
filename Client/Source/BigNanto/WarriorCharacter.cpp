// Fill out your copyright notice in the Description page of Project Settings.


#include "WarriorCharacter.h"
#include "PlayerCharacterAnim.h"

AWarriorCharacter::AWarriorCharacter()
{
	CharacterClass = ECharacterClass::EWarrior;
}

void AWarriorCharacter::SpecialAbility()
{
	Defend();
}

void AWarriorCharacter::StopSpecialAbility()
{
	StopDefend();
}

void AWarriorCharacter::Defend()
{
	if (CurrentState != ECharacterState::EIdle)
		return;

	SetCurrentState(ECharacterState::EDefend);
	if (AnimInstance)
		AnimInstance->bIsDefending = true;
}

void AWarriorCharacter::StopDefend()
{
	if (CurrentState == ECharacterState::EDefend)
		SetCurrentState(ECharacterState::EIdle);
	if (AnimInstance)
		AnimInstance->bIsDefending = false;
}
