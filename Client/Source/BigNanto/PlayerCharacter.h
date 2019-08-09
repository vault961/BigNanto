// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Runtime/UMG/Public/UMG.h"
#include "PlayerCharacter.generated.h"
#define NAMELEN 20

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

// �׼�
UENUM()
enum class ECharacterAction : uint8
{
	EA_DefendHit,
	EA_StopAttack,
	EA_Attack,
	EA_Defend,
	EA_Hit,
	EA_Jump,
	EA_SpecialAbility,
	EA_StopSpecialAbility,
	EA_Die,
	EA_Move,
	EA_StopMove,
};

// ĳ���� ����
UENUM()
enum class ECharacterClass : uint8
{
	EUnknown,	// �� �� ���� Ŭ����
	EWarrior,	// ����
	EWizard,	// ������
};

UCLASS()
class UBigUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class APlayerCharacter* PlayerCharacterRef;

	UFUNCTION(BlueprintCallable)
	class APlayerCharacter* GetPlayerRef() { return PlayerCharacterRef; }
};

UCLASS(Blueprintable, BlueprintType)
class BIGNANTO_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:	
	// Sets default values for this character's properties
	APlayerCharacter();
	~APlayerCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ī�޶� ������ ������ ��
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class USpringArmComponent* CameraBoom;

	// ���̵� �� ī�޶� ������Ʈ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class UCameraComponent* SideViewCameraComponent;

	// ĳ���� ���� UI (�������ۼ�Ʈ, �÷��̾� ���� ǥ��)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	class UWidgetComponent* PlayerUI;

	// �÷��̾� �̸�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CharacterInfo)
	FString PlayerName;
	wchar_t NameArr[11];	// �ѱ� �Է��� ���ؼ� ������ �迭

	// ĳ���� ������ �ۼ�Ʈ
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CharacterInfo)
	float DamagePercent;

	// ���� ī����
	UPROPERTY(EditAnywhere, Category = CharacterInfo)
	uint8 JumpCount;

	// ��������
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CharacterInfo)
	uint8 LifeCount;

	// �ִ� �ν��Ͻ� ���۷���
	class UPlayerCharacterAnim* AnimInstance;
	// ���� �ν��Ͻ� ���۷���
	class UBigNantoGameInstance* GameInstance;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UCameraShake> CameraShake;

	// �������� ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CharacterInfo)
	class AWeapon* Weapon;
	bool GetWeaponActive() const;
	void SetWeaponActive(bool bIsActive);

	// �ǰ� ��ƼŬ
	UParticleSystem* HitParticle;

	// ��� ��ƼŬ(������2D)
	
	// ĳ���� �������
	UPROPERTY(VisibleAnywhere, Category = CharacterInfo)
	ECharacterState CurrentState;
	ECharacterState GetCurrentState() const;
	void SetCurrentState(ECharacterState NewState);

	// ĳ���� ����
	UPROPERTY(VisibleAnywhere, Category = CharacterInfo)
	ECharacterClass CharacterClass;

	int32 SendDelay;
	char body[50];
	char anibody;
	bool IsMine;
	uint8 NewDir;
	uint8 PlayerDir;
	uint32 MyID;			// �÷��̾� ���̵�

	FVector NewLocation;
	float NewYaw;

	FVector PlayerLocation;
	float PlayerYaw;

	// ������ ��ġ, ���� ����
	FVector UpdatedLocation;
	FRotator UpdatedRotation;

	UFUNCTION()
	void UpdateLocation(FVector New, uint8 Dir);
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
	
	// �ǰ� �� �˹� �Լ�
	virtual void HitandKnockback(FVector HitDirection, float HitDamage);

	// ����
	UFUNCTION()
	virtual void Attack();
	UFUNCTION()
	virtual void StopAttack();

	// Ư���ɷ� �� �Լ�
	virtual void CallSpecialAbility();
	virtual void CallStopSpecialAbility();

	// Ư���ɷ�
	UFUNCTION()
	virtual void SpecialAbility();
	UFUNCTION()
	virtual void StopSpecialAbility();

	// ���
	UFUNCTION(BlueprintCallable)
	virtual void Die();

	// ���ƿ� ����Ʈ ����
	virtual void PlayRingOutEffect();
};
