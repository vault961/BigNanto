// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

// ĳ���� ����
UENUM()
enum class ECharacterState : uint8
{
	EIdle,			// ����� ����
	EJump,			// ����
	EAttack,		// ���� ��
	EDefend,		// ���
	EHit,			// �´� ��
	EChanneling,	// ���� ��
};

// ĳ���� ����
UENUM()
enum class ECharacterClass : uint8
{
	ENULLCLASS,	// Ŭ���� ������
	EWarrior,	// ����
	EWizard,	// ������
};

UCLASS()
class BIGNANTO_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:	
	// Sets default values for this character's properties
	APlayerCharacter();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ī�޶� ������ ������ ��
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class USpringArmComponent* CameraBoom;

	// ���̵� �� ī�޶� ������Ʈ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class UCameraComponent* SideViewCameraComponent;

	// ĳ�����̸�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CharacterInfo)
	FName CharacterName;

	// ĳ���� ������ �ۼ�Ʈ
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CharacterInfo)
	float DamagePercent;

	// ���� ī����
	UPROPERTY(EditAnywhere, Category = CharacterInfo)
	uint8 JumpCount;

	// ��������
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CharacterInfo)
	uint8 LifeCount;

	// �ִ� �ν��Ͻ�
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Animation)
	class UPlayerCharacterAnim* AnimInstance;

	// �������� ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CharacterInfo)
	class AWeapon* Weapon;
	bool GetWeaponActive() const;
	void SetWeaponActive(bool bIsActive);

	// �ǰ� ��ƼŬ
	UPROPERTY()
	UParticleSystem* HitParticle;
	
	// ĳ���� �������
	UPROPERTY(VisibleAnywhere, Category = CharacterInfo)
	ECharacterState CurrentState;
	ECharacterState GetCurrentState() const;
	void SetCurrentState(ECharacterState NewState);

	UPROPERTY(VisibleAnywhere, Category = CharacterInfo)
	ECharacterClass CharacterClass;

	class UBigNantoGameInstance* GameInstance;
	int32 SendDelay;
	char body[100];
	bool IsMine;
	FVector NewLocation;
	FVector PlayerLocation;
	FVector destination;
	
	UFUNCTION()
	void UpdatePosition(FVector New);
	UFUNCTION()
	void UpdateStatus();

	// �浹����
	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult &SweepResult);

	// ����
	UFUNCTION()
	void DoJump();

	// ������ �̵�
	UFUNCTION()
	void MoveRight(float val);

	// ���� �ǰ�
	UFUNCTION(BlueprintCallable)
	virtual void AttackHit(AWeapon* OverlappedWeapon);

	// ���� �ǰ�
	virtual void AbilityHit(class AWeapon_MagicWand* OverlappedAbility);
	
	virtual void HitandKnockback(FVector HitDirection, float HitDamage);

	// ����
	UFUNCTION()
	virtual void Attack();
	UFUNCTION()
	virtual void StopAttack();

	// Ư���ɷ�
	UFUNCTION()
	virtual void SpecialAbility();
	UFUNCTION()
	virtual void StopSpecialAbility();
};
