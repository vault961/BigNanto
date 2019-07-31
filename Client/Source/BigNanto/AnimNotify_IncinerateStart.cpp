// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_IncinerateStart.h"
#include "Components/SkeletalMeshComponent.h"
#include "WizardCharacter.h"
#include "Weapon_MagicWand.h"

void UAnimNotify_IncinerateStart::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	AWizardCharacter* WizardCharacter = Cast<AWizardCharacter>(MeshComp->GetOwner());
	if (nullptr == WizardCharacter)
		return;

	AWeapon_MagicWand* MagicWand = Cast<AWeapon_MagicWand>(WizardCharacter->Weapon);

	if (MagicWand)
	{
		MagicWand->bIsIncinerateOn = true;
	}
}