// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_JumpEnd.h"
#include "Components/SkeletalMeshComponent.h"
#include "PlayerCharacter.h"

void UAnimNotify_JumpEnd::Notify(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
	APlayerCharacter *PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner());
	if (nullptr == PlayerCharacter)
		return;

	PlayerCharacter->JumpCount = 0;
	PlayerCharacter->SetCurrentState(ECharacterState::EIdle);
}
