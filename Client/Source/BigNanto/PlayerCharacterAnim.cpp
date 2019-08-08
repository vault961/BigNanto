// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterAnim.h"
#include "GameFramework/PawnMovementComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "PlayerCharacter.h"
#include "Engine/Engine.h"
#include "BigNantoGameInstance.h"

UPlayerCharacterAnim::UPlayerCharacterAnim()
{
	// 맞는 몽타주
	static ConstructorHelpers::FObjectFinder<UAnimMontage> GetHitMontageAsset(TEXT("/Game/Characters/Animations/GetHit_Montage"));
	if (GetHitMontageAsset.Succeeded()) 
		HitMontage = GetHitMontageAsset.Object;

	// 공격 몽타주
	static ConstructorHelpers::FObjectFinder<UAnimMontage> AttackMontageAsset(TEXT("/Game/Characters/Animations/Attack_Montage"));
	if (AttackMontageAsset.Succeeded())
		AttackMontage = AttackMontageAsset.Object;

	// 소각마법 몽타주
	static ConstructorHelpers::FObjectFinder<UAnimMontage> IncinerateMontageAsset(TEXT("/Game/Characters/Animations/Incinerate_Montage"));
	if (IncinerateMontageAsset.Succeeded())
		IncinerateMontage = IncinerateMontageAsset.Object;
}

UPlayerCharacterAnim::~UPlayerCharacterAnim()
{
	//UE_LOG(LogTemp, Log, TEXT("캐릭터 애님인스턴스 호출"));
}

void UPlayerCharacterAnim::NativeUpdateAnimation(float DeltaTime)
{
	PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());

	if (PlayerCharacter)
	{
		bIsFalling = PlayerCharacter->GetMovementComponent()->IsFalling();
		MoveSpeed = PlayerCharacter->GetVelocity().Size();

		// 내 캐릭터만 움직임 여부 반영
		if (PlayerCharacter->IsMine == true)
		{
			if (MoveSpeed != 0)
			{
				if (bIsMoving == false)
				{
					char anibody = (char)ECharacterAction::EA_Move;
					PlayerCharacter->GameInstance->SendMessage(
						PACKET_TYPE::UPDATESTATE, &anibody, 1);

				}
				bIsMoving = true;
			}
			else
			{
				if (bIsMoving == true)
				{
					char anibody = (char)ECharacterAction::EA_StopMove;
					PlayerCharacter->GameInstance->SendMessage(
						PACKET_TYPE::UPDATESTATE, &anibody, 1);

				}
				bIsMoving = false;
			}
		}
		
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
