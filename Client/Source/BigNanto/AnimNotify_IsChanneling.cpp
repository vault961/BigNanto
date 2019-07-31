// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_IsChanneling.h"
#include "Components/SkeletalMeshComponent.h"
#include "PlayerCharacterAnim.h"
#include "Weapon_MagicWand.h"
#include "WizardCharacter.h"

void UAnimNotify_IsChanneling::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	AWizardCharacter* WizardCharacter = Cast<AWizardCharacter>(MeshComp->GetOwner());
	if (nullptr == WizardCharacter)
		return;

	UPlayerCharacterAnim* AnimInstance = WizardCharacter->AnimInstance;
	if (!AnimInstance->bIsChanneling)
	{
		AWeapon_MagicWand* MagicWand = Cast<AWeapon_MagicWand>(WizardCharacter->Weapon);
		if (MagicWand)
		{
			AnimInstance->Montage_SetNextSection(TEXT("Incinerate_Loop"), TEXT("Incinerate_End"));
			MagicWand->bIsIncinerateOn = false;
		}
	}
}
