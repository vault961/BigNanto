// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_ComboAttack3.h"
#include "Components/SkeletalMeshComponent.h"
#include "PlayerCharacter.h"
#include "PlayerCharacterAnim.h"

void UAnimNotify_ComboAttack3::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	APlayerCharacter *PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner());
	if (nullptr == PlayerCharacter)
		return;

	UPlayerCharacterAnim* AnimInstance = PlayerCharacter->AnimInstance;

	if (!AnimInstance->bIsAttacking)
	{
		AnimInstance->Montage_SetNextSection(TEXT("ComboAttack03"), TEXT("ComboEnd"));
		PlayerCharacter->SetCurrentState(ECharacterState::EIdle);
		PlayerCharacter->SetWeaponActive(false);
	}
	else
		PlayerCharacter->SetWeaponActive(true);
}