// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_HitEnd.h"
#include "Components/SkeletalMeshComponent.h"
#include "PlayerCharacter.h"

void UAnimNotify_HitEnd::Notify(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
	APlayerCharacter *PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner());
	if (nullptr == PlayerCharacter)
		return;

	PlayerCharacter->SetCurrentState(ECharacterState::EIdle);
}