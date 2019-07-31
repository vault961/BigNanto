// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_AttackStart.h"
#include "Components/SkeletalMeshComponent.h"
#include "PlayerCharacter.h"

void UAnimNotify_AttackStart::Notify(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
	APlayerCharacter *PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner());
	if (nullptr == PlayerCharacter)
		return;

	PlayerCharacter->SetWeaponActive(true);
	PlayerCharacter->SetCurrentState(ECharacterState::EAttack);
}