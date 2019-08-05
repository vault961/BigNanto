// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PlayerCharacterAnim.h"
#include "Weapon.h"
#include "UObject/ConstructorHelpers.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon_MagicWand.h"
#include "BigNantoGameInstance.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// ĸ�� ������Ʈ
	GetCapsuleComponent()->InitCapsuleSize(40.f, 90.f);													// ĳ���� ĸ�� ������
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::BeginOverlap);	// ĳ���� �浹 �Լ� ����
	GetCapsuleComponent()->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_Yes;							// ĳ���� ���� ���� �ö� �� �� �ִ���
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));										// ĳ���� �浹 ä�� = Pawn Ÿ��
	GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));											// ĳ���� �Ž� �浹 ä�� = NoCollision

	// ��Ʈ�ѷ� ȸ�� ��� ����
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	// ī�޶�� ���� ���Ŀ� ��Ʈ ������Ʈ�� �ٿ���
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bAbsoluteRotation = true;											// ĳ���Ͱ� ȸ���ص� ī�޶� ȸ���� ���� ���� ����
	CameraBoom->bDoCollisionTest = false;											
	CameraBoom->TargetArmLength = 1000.f;											// ī�޶� �Ÿ�
	CameraBoom->SocketOffset = FVector(0.f, 0.f, 75.f);								// ī�޶� ������ ��ġ (���� ����
	CameraBoom->RelativeRotation = FRotator(0.f, 180.f, 0.f);						// ī�޶� ȸ������

	// ī�޶���� �� �տ� �ٿ��ֱ�
	SideViewCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("SideViewCamera"));
	SideViewCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	SideViewCameraComponent->bUsePawnControlRotation = false;									// ī�޶�� ȸ������ ����

	// ĳ���� �����Ʈ ����
	//GetCharacterMovement()->bOrientRotationToMovement = true; // �̵��ϴ� �������� ȸ��
	//GetCharacterMovement()->RotationRate = FRotator(0.f, 1000.f, 0.f); // ȸ���ϴ� ����
	GetCharacterMovement()->GravityScale = 3.f;		// �߷°�
	GetCharacterMovement()->AirControl = 0.8f;		// ���߿��� ��Ʈ�� �� �� �ִ� ��
	GetCharacterMovement()->JumpZVelocity = 1300.f; // ������
	GetCharacterMovement()->GroundFriction = 5.f;	// ����
	GetCharacterMovement()->MaxWalkSpeed = 1000.f;	// �ִ�ӵ�
	GetCharacterMovement()->MaxFlySpeed = 1000.f;	// �ִ� ���� �ӵ�

	// ĳ���� ���� �ʱ�ȭ
	DamagePercent = 0.f;
	CurrentState = ECharacterState::EIdle;
	JumpCount = 0;
	LifeCount = 0;
	CharacterClass = ECharacterClass::EUnknown;

	// ��Ʈ ��ƼŬ
	static ConstructorHelpers::FObjectFinder<UParticleSystem> HitParticleAsset(TEXT("/Game/StarterContent/Particles/P_Explosion"));
	if (HitParticleAsset.Succeeded())
		HitParticle = HitParticleAsset.Object;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// �ִ� �ν��Ͻ� �ҷ�����
	if (GetMesh())
	{
		AnimInstance = Cast<UPlayerCharacterAnim>(GetMesh()->GetAnimInstance());
	}

	// ���� �޾��ֱ�
	UChildActorComponent* ChildActorComp = FindComponentByClass<UChildActorComponent>();
	if (ChildActorComp)
	{
		Weapon = Cast<AWeapon>(ChildActorComp->GetChildActor());
		if(Weapon)
			Weapon->WeaponOwner = this;
	}
	
	GameInstance = Cast<UBigNantoGameInstance>(GetGameInstance());
	
	//SetActorTickInterval(0.2f);
	PlayerLocation = GetActorLocation();

	//NewLocation.Y = PlayerLocation.Y;
	//NewLocation.Z = PlayerLocation.Z;
	SendDelay = 0;
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetActorLocation().X != 0.f)
	{
		FVector ActorLocation = GetActorLocation();
		ActorLocation.X = 0.f;
		SetActorLocation(ActorLocation);
	}

	PlayerLocation = GetActorLocation();
	
	// ����
	// �÷��̾� �ƴϸ�
	if (!IsMine)
	{
		UpdatedLocation = FMath::VInterpTo(PlayerLocation, NewLocation, DeltaTime, 10.f);
		
		AddMovementInput(FVector(0.f, -1.f, 0.f), 1.f);
		FRotator NewRotation;
		if (UpdatedLocation.Y > PlayerLocation.Y) {
			NewRotation = FRotator(0, 1, 0);
		}
		else {
			NewRotation = FRotator(0, -1, 0);
		}

		//FQuat QuatRotation = FQuat(NewRotation);
		//AddActorLocalRotation(QuatRotation, false, 0, ETeleportType::None);
		//destination = NewLocation;
		//SetActorLocation(UpdatedLocation, false);

	}
	else {
		// �� ��ġ �۽�
		SendDelay += 1;
		if (SendDelay == 3) {

			memcpy(body, &PlayerLocation.Y, sizeof(PlayerLocation.Y));
			memcpy(body + 4, &PlayerLocation.Z, sizeof(PlayerLocation.Z));
			GameInstance->SendMessage(PACKET_TYPE::UPDATELOCATION, body, 8);
			SendDelay = 0;
		}
	}

}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// �����÷��� Ű ���ε�
	check(PlayerInputComponent);
	// �̵� 
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);
	// ����
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::DoJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	// ����
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &APlayerCharacter::Attack);
	PlayerInputComponent->BindAction("Attack", IE_Released, this, &APlayerCharacter::StopAttack);
	// ����
	PlayerInputComponent->BindAction("SpecialAbility", IE_Pressed, this, &APlayerCharacter::SpecialAbility);
	PlayerInputComponent->BindAction("SpecialAbility", IE_Released, this, &APlayerCharacter::StopSpecialAbility);
}

bool APlayerCharacter::GetWeaponActive() const
{
	return Weapon ? Weapon->bIsActive : false;
}

void APlayerCharacter::SetWeaponActive(bool bIsActive)
{
	if (Weapon)
		Weapon->bIsActive = bIsActive;
}

ECharacterState APlayerCharacter::GetCurrentState() const
{
	return CurrentState;
}

void APlayerCharacter::SetCurrentState(ECharacterState NewState)
{
	CurrentState = NewState;
}

void APlayerCharacter::UpdateLocation(FVector New)
{
	NewLocation = New;
	UE_LOG(LogTemp, Warning, TEXT("update position %f %f"), New.Y, New.Z);
}

void APlayerCharacter::UpdateStatus()
{
	//UE_LOG(LogTemp, Warning, TEXT("update status"));
}

void APlayerCharacter::BeginOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	// �浹�� ���� ��� �迭�� �ֱ�
	TArray<AActor*> CollectedActors;
	GetCapsuleComponent()->GetOverlappingActors(CollectedActors);

	for (int32 iCollected = 0; iCollected < CollectedActors.Num(); ++iCollected)
	{
		// �浹�� ���Ͱ� �����ΰ�� & �� ���Ⱑ �ڽ��� ���Ⱑ �ƴ� ��� & ���Ⱑ Ȱ��ȭ �Ǿ��ִ� ���
		AWeapon* const OverlappedWeapon = Cast<AWeapon>(CollectedActors[iCollected]);
		if (OverlappedWeapon && (Weapon != OverlappedWeapon))
		{
			if (OverlappedWeapon->bIsActive)
			{
				// �ǰ� �Լ� ȣ��
				AttackHit(OverlappedWeapon);
			}
			AWeapon_MagicWand* const OverlappedAbility = Cast<AWeapon_MagicWand>(OverlappedWeapon);
			if (OverlappedAbility && OverlappedAbility->bIsIncinerateOn)
			{
				AbilityHit(OverlappedAbility);
			}
		}
	}
}

void APlayerCharacter::DoJump()
{
	if (IsMine)
	{
		if (CurrentState != ECharacterState::EIdle && CurrentState != ECharacterState::EJump)
			return;

		// �� �ϰ��, defenthit �ִϸ��̼� ����
		anibody[0] = (char)ECharacterAction::EA_Jump;
		GameInstance->SendMessage(PACKET_TYPE::UPDATESTATE, anibody, 1);

		// ���� ������ ���� ī��Ʈ �ʱ�ȭ
		// �ִ� ��Ƽ���̿����� �ʱ�ȭ ���ֱ� �ϴµ� Ȥ�� ����
		if (GetCharacterMovement()->IsMovingOnGround() == true)
		{
			JumpCount = 0;
		}

		// ���� ī��Ʈ 2�̸��Ͻ� ���� ����(�ִ� 2�� ����)
		if (JumpCount < 2)
		{
			SetCurrentState(ECharacterState::EJump);
			LaunchCharacter(FVector(0.f, 0.f, 1.f) * GetCharacterMovement()->JumpZVelocity, false, true);
			JumpCount++;
		}
	}
	else
	{
		SetCurrentState(ECharacterState::EJump);
			LaunchCharacter(FVector(0.f, 0.f, 1.f) * GetCharacterMovement()->JumpZVelocity, false, true);
	}
	
}

void APlayerCharacter::MoveRight(float val)
{
	//UE_LOG(LogTemp, Log, TEXT("Input Value : %f"), val);
	if ((CurrentState == ECharacterState::EIdle) || (CurrentState == ECharacterState::EJump))
	{
		if(0 != val)
			SetActorRelativeRotation(FRotator(0.f, -90.f * FMath::RoundFromZero(val), 0.f ));

		// �Է¹��� �������� �̵�
		AddMovementInput(FVector(0.f, -1.f, 0.f), val);
	}
	
}

void APlayerCharacter::AttackHit(AWeapon* OverlappedWeapon)
{
	// �ǰ��� ĳ������ ���� ����
	FVector EnemyForwardVector = OverlappedWeapon->WeaponOwner->GetActorForwardVector();
	//UE_LOG(LogTemp, Log, TEXT("EnemyForwardVector : %s"), *EnemyForwardVector.ToString());

	// ���� ���� ���� ����
	// ���� ���� ����� ���� �� �ڿ� ����, ������ ���� �� �տ� ����
	float HitDot = FVector::DotProduct(GetActorForwardVector(), EnemyForwardVector);
	
	if (AnimInstance)
	{
		// ���� �� �տ� �ְ� ��� �� �Ͻ� (���� ��Ʈ �ִϸ��̼�)
		if ((HitDot < 0 ) && CurrentState == ECharacterState::EDefend)
		{
			// �� �ϰ��, defenthit �ִϸ��̼� ����
			if (IsMine)
			{
				anibody[0] = (char)ECharacterAction::EA_DefendHit;
				GameInstance->SendMessage(PACKET_TYPE::UPDATESTATE, anibody, 1);
			}

			AnimInstance->PlayDefendHit();
		}
		// ���� �� �ڿ� �ְų� ��� ���� �ƴҽ� (�Ϲ� ��Ʈ �ִϸ��̼�)
		else
		{
			HitandKnockback(EnemyForwardVector, OverlappedWeapon->AttackDamage);
		}
	}
}

void APlayerCharacter::AbilityHit(AWeapon_MagicWand * OverlappedAbility)
{
	FVector EnemyForwardVector = OverlappedAbility->WeaponOwner->GetActorForwardVector();
	HitandKnockback(EnemyForwardVector, OverlappedAbility->IncinerateDamage);
}

void APlayerCharacter::HitandKnockback(FVector HitDirection, float HitDamage)
{
	// �� �ϰ�� Hit �ִϸ��̼� ����, hit damage ó��
	if (IsMine)
	{
		anibody[0] = (char)ECharacterAction::EA_Hit;
		GameInstance->SendMessage(PACKET_TYPE::UPDATESTATE, anibody, 1);

		// ������ �ۼ�Ʈ�� ��Ʈ ������ �߰�
		DamagePercent += HitDamage;
		// ���� ���� �������� �˹�
		LaunchCharacter(HitDirection * (HitDamage * DamagePercent + 100.f), true, true);

	}


	// �����ൿ �ߴ��ϰ� EHit ���·� �ٲ��ֱ�
	StopAttack();
	StopSpecialAbility();
	SetCurrentState(ECharacterState::EHit);
	AnimInstance->PlayGetHit();
	

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, GetActorTransform());
}
void APlayerCharacter::Attack()
{
	// ���ϰ�� attack �ִϸ��̼� ����
	if (IsMine)
	{
		if (CurrentState != ECharacterState::EIdle)
			return;

		anibody[0] = (char)ECharacterAction::EA_Attack;
		GameInstance->SendMessage(PACKET_TYPE::UPDATESTATE, anibody, 1);
	}

	if (AnimInstance)
	{
		AnimInstance->PlayAttack();
		AnimInstance->bIsAttacking = true;
		SetCurrentState(ECharacterState::EAttack);
	}
}

void APlayerCharacter::StopAttack()
{
	// �� �ϰ�� stopattack ��ȣ ����.
	if (IsMine)
	{
		anibody[0] = (char)ECharacterAction::EA_StopAttack;
		GameInstance->SendMessage(PACKET_TYPE::UPDATESTATE, anibody, 1);
	}

	if (AnimInstance)
	{
		AnimInstance->bIsAttacking = false;
	}
}

void APlayerCharacter::SpecialAbility()
{
	UE_LOG(LogTemp, Warning, TEXT("This character has no ability!!!"));
}

void APlayerCharacter::StopSpecialAbility()
{
	// THERE IS NO WAY STOP SPECIAL ABILITY OF THIS CHARACTER!!!
}
