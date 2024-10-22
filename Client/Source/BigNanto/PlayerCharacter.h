// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Runtime/UMG/Public/UMG.h"
#include "PlayerCharacter.generated.h"
#define NAMELEN 20

// 캐릭터 상태
UENUM()
enum class ECharacterState : uint8
{
	EIdle,			// 평범한 상태
	EJump,			// 점프
	EAttack,		// 공격 중
	EDefend,		// 방어
	EHit,			// 맞는 중
	EChanneling,	// 집중 중
	EDead,			// 사망
};

// 액션
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
	EA_IgnorePlatform,
	EA_BlockPlatform,
};

// 캐릭터 직업
UENUM()
enum class ECharacterClass : uint8
{
	EUnknown,	// 알 수 없는 클래스
	EWarrior,	// 전사
	EWizard,	// 마법사
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

	// 카메라를 부착할 스프링 암
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class USpringArmComponent* CameraBoom;

	// 사이드 뷰 카메라 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class UCameraComponent* SideViewCameraComponent;

	// 캐릭터 상태 UI (데미지퍼센트, 플레이어 네임 표시)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	class UWidgetComponent* PlayerUI;

	// 플레이어 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CharacterInfo)
	FString PlayerName;
	wchar_t NameArr[11];	// 한글 입력을 위해서 만들어둔 배열

	// 캐릭터 데미지 퍼센트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CharacterInfo)
	float DamagePercent;

	// 점프 카운터
	UPROPERTY(EditAnywhere, Category = CharacterInfo)
	uint8 JumpCount;

	// 남은생명
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CharacterInfo)
	uint8 LifeCount;

	// 애님 인스턴스 레퍼런스
	class UPlayerCharacterAnim* AnimInstance;
	// 게임 인스턴스 레퍼런스
	class UBigNantoGameInstance* GameInstance;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UCameraShake> CameraShake;

	// 소유중인 무기
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = CharacterInfo)
	class AWeapon* Weapon;
	bool GetWeaponActive() const;
	void SetWeaponActive(bool bIsActive);

	// 피격 파티클
	UParticleSystem* HitParticle;

	// 캐릭터 현재상태
	UPROPERTY(VisibleAnywhere, Category = CharacterInfo)
	ECharacterState CurrentState;
	ECharacterState GetCurrentState() const;
	void SetCurrentState(ECharacterState NewState);

	// 캐릭터 직업
	UPROPERTY(VisibleAnywhere, Category = CharacterInfo)
	ECharacterClass CharacterClass;

	int32 SendDelay;
	char body[50];
	char anibody;
	bool IsMine;
	uint8 NewDir;
	uint8 PlayerDir;

	uint32 PlayerID;		// 플레이어 아이디
	uint32 LastHitOwner;	// 마지막으로 나를 공격한 사람

	// 플레이어 카운트
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 KillCount;		

	FVector NewLocation;
	float NewYaw;

	FVector PlayerLocation;
	float PlayerYaw;

	// 보정할 위치, 방향 수신
	FVector UpdatedLocation;
	FRotator UpdatedRotation;

	UPROPERTY(VisibleAnywhere)
	bool bIgnorePlatform;

	// 이전 틱에서 캐릭터 위치
	float DeltaZ;
	

	// ===================== 함수 =====================

	void CorrectLocation(float DeltaTime);
	void UpdateLocation(FVector New, uint8 Dir);

	// 충돌감지
	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult &SweepResult);

	UFUNCTION()
	void EndOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 점프
	UFUNCTION()
	void DoJump();

	// 오른쪽 이동
	UFUNCTION()
	void MoveRight(float val);

	// 물리 피격
	UFUNCTION(BlueprintCallable)
	virtual void AttackHit(AWeapon* OverlappedWeapon);

	// 마법 피격
	virtual void AbilityHit(class AWeapon_MagicWand* OverlappedAbility);
	
	// 피격 및 넉백 함수
	virtual void HitandKnockback(FVector HitDirection, float HitDamage);

	// 공격
	UFUNCTION()
	virtual void Attack();
	UFUNCTION()
	virtual void StopAttack();

	// 특수능력 콜 함수
	virtual void CallSpecialAbility();
	virtual void CallStopSpecialAbility();

	// 특수능력
	UFUNCTION()
	virtual void SpecialAbility();
	UFUNCTION()
	virtual void StopSpecialAbility();

	// 사망
	UFUNCTION(BlueprintCallable)
	virtual void Die();

	// 링아웃 이펙트 생성
	virtual void PlayRingOutEffect();

	void SetIgnorePlatform();
	void SetBlockPlatform();
	void IgnorePlatform(bool bIsIgnore);
};
