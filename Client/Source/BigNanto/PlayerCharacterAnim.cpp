// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterAnim.h"
#include "GameFramework/PawnMovementComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "PlayerCharacter.h"
#include "Engine/Engine.h"

UPlayerCharacterAnim::UPlayerCharacterAnim()
{
	// 嘎绰 根鸥林
	static ConstructorHelpers::FObjectFinder<UAnimMontage> GetHitMontageAsset(TEXT("/Game/Characters/Animations/GetHit_Montage"));
	if (GetHitMontageAsset.Succeeded()) 
		HitMontage = GetHitMontageAsset.Object;

	// 傍拜 根鸥林
	static ConstructorHelpers::FObjectFinder<UAnimMontage> AttackMontageAsset(TEXT("/Game/Characters/Animations/Attack_Montage"));
	if (AttackMontageAsset.Succeeded())
		AttackMontage = AttackMontageAsset.Object;

	// 家阿付过 根鸥林
	static ConstructorHelpers::FObjectFinder<UAnimMontage> IncinerateMontageAsset(TEXT("/Game/Characters/Animations/Incinerate_Montage"));
	if (IncinerateMontageAsset.Succeeded())
		IncinerateMontage = IncinerateMontageAsset.Object;
}

void UPlayerCharacterAnim::NativeUpdateAnimation(float DeltaTime)
{
	PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());

	if (PlayerCharacter)
	{
		bIsFalling = PlayerCharacter->GetMovementComponent()->IsFalling();
		MoveSpeed = PlayerCharacter->GetVelocity().Size();
	}
}

void UPlayerCharacterAnim::PlayGetHit()
{
	Montage_Play(HitMontage);
}

void UPlayerCharacterAnim::PlayDefendHit()
{
	Montage_Play(HitMontage);
	Montage_JumpToSection("DefendHit", HitMontage);
}

void UPlayerCharacterAnim::PlayAttack()
{
	if (AttackMontage)
	{
		Montage_Play(AttackMontage);
	}
}

void UPlayerCharacterAnim::PlayIncinerate()
{
	if (IncinerateMontage)
	{
		Montage_Play(IncinerateMontage);
	}
}
