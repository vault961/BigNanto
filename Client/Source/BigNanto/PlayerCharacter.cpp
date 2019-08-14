// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "Animation/AnimInstance.h"
#include "RingOutExplosion.h"
#include "BigNantoGameInstance.h"
#include "BigNantoGameModeBase.h"
#include "BigNantoPlayerController.h"
#include "CenterViewPawn.h"
#include "CharacterSpawner.h"
#include "Components/WidgetComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "PlayerCharacterAnim.h"
#include "UObject/ConstructorHelpers.h"
#include "Weapon.h"
#include "Weapon_MagicWand.h"
#include "BigNantoCameraShake.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// 캡슐 컴포넌트
	GetCapsuleComponent()->InitCapsuleSize(40.f, 90.f);													// 캐릭터 캡슐 사이즈
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::BeginOverlap);	// 캐릭터 충돌 함수 위임
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::EndOverlap);		
	GetCapsuleComponent()->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;							// 캐릭터 위에 누가 올라 설 수 있는지
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));										// 캐릭터 충돌 채널 = Pawn 타입
	GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));											// 캐릭터 매쉬 충돌 채널 = NoCollision

	// 컨트롤러 회전 사용 안함
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	//--------------------- 슈퍼 스매시 브라더스 캠으로 전환 후 개인 카메라는 생성 안함 --------------------- 
	//// 카메라붐 생성 이후에 루트 컴포넌트에 붙여줌
	//CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	//CameraBoom->SetupAttachment(RootComponent);
	//CameraBoom->bAbsoluteRotation = true;											// 캐릭터가 회전해도 카메라 회전에 영향 받지 않음
	//CameraBoom->bDoCollisionTest = false;											
	//CameraBoom->TargetArmLength = 1000.f;											// 카메라 거리
	//CameraBoom->SocketOffset = FVector(0.f, 0.f, 75.f);								// 카메라 오프셋 위치 (높이 조정
	//CameraBoom->RelativeRotation = FRotator(0.f, 180.f, 0.f);						// 카메라 회전각도

	//// 카메라생성 후 붐에 붙여주기
	//SideViewCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("SideViewCamera"));
	//SideViewCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	//SideViewCameraComponent->bUsePawnControlRotation = false;									// 카메라는 회전하지 않음
	// ------------------------------------------------------------------------------------------------------

	// 캐릭터 무브먼트 설정
	//GetCharacterMovement()->bOrientRotationToMovement = true;				// 이동하는 방향으로 회전 -> 회전은 굳이 안넣어줘도 될 듯
	//GetCharacterMovement()->RotationRate = FRotator(0.f, 1000.f, 0.f);	// 회전하는 비율 
	GetCharacterMovement()->GravityScale = 3.f;		// 중력값
	GetCharacterMovement()->AirControl = 0.8f;		// 공중에서 컨트롤 할 수 있는 힘
	GetCharacterMovement()->JumpZVelocity = 1300.f; // 점프력
	GetCharacterMovement()->GroundFriction = 10.f;	// 마찰
	GetCharacterMovement()->MaxWalkSpeed = 1000.f;	// 최대속도
	GetCharacterMovement()->MaxFlySpeed = 1000.f;	// 최대 공중 속도

	// 오토포세스AI를 PlacedInWorldOrSpawned로 바꿔줌
	// 이렇게 하면 다른 클라이언트에서도 AddMovementInput을 사용할 수 있음 ㅇㅅㅇ!
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 캐릭터 정보 초기화
	DamagePercent = 0.f;
	CurrentState = ECharacterState::EIdle;
	JumpCount = 0;
	LifeCount = 0;
	NewDir = 0;
	PlayerDir = 0;
	KillCount = 0;
	LastHitOwner = 0;
	bIgnorePlatform = false;
	CharacterClass = ECharacterClass::EUnknown;
	SendDelay = 0;

	// 히트 파티클
	static ConstructorHelpers::FObjectFinder<UParticleSystem> HitParticleAsset(TEXT("/Game/StarterContent/Particles/P_Explosion"));
	if (HitParticleAsset.Succeeded())
		HitParticle = HitParticleAsset.Object;

	// 캐릭터 UI
	PlayerUI = CreateDefaultSubobject<UWidgetComponent>(TEXT("PlayerUI"));
	static ConstructorHelpers::FClassFinder<UUserWidget> PlayerUIAsset(TEXT("/Game/UMG/PlayerUI"));
	if (PlayerUIAsset.Succeeded())
	{
		PlayerUI->SetWidgetClass(PlayerUIAsset.Class);
	}

	
	PlayerUI->SetupAttachment(RootComponent);				// 캐릭터 UI를 루트 컴포넌트에 부착
	PlayerUI->RelativeLocation = FVector(0.f, 0.f, 150.f);	// UI 위치
	PlayerUI->SetWidgetSpace(EWidgetSpace::Screen);			// 위젯 공간 설정 (스크린으로)

	CameraShake = UBigNantoCameraShake::StaticClass();		// 카메라 쉐이크

	SetActorScale3D(FVector::OneVector * 0.7f);				// 캐릭터 스케일
}

APlayerCharacter::~APlayerCharacter()
{
	UE_LOG(LogTemp, Log, TEXT("플레이어 '%s' 소멸자 호출"), *PlayerName);
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 시작하자마자 옆으로 보게 
	SetActorRelativeRotation(FRotator(0.f, 90.f, 0.f));

	// 애님 인스턴스 불러오기
	if (GetMesh())
	{
		AnimInstance = Cast<UPlayerCharacterAnim>(GetMesh()->GetAnimInstance());
	}

	// 무기 달아주기
	UChildActorComponent* ChildActorComp = FindComponentByClass<UChildActorComponent>();
	if (ChildActorComp)
	{
		Weapon = Cast<AWeapon>(ChildActorComp->GetChildActor());
		if (Weapon)
			Weapon->WeaponOwner = this;
	}

	// 게임 인스턴스 가져오기
	GameInstance = Cast<UBigNantoGameInstance>(GetGameInstance());

	// 위젯에게 플레이어캐릭터 레퍼런스 전달
	Cast<UBigUserWidget>(PlayerUI->GetUserWidgetObject())->PlayerCharacterRef = this;

	//UE_LOG(LogTemp, Log, TEXT("My Owner is : %s"), *PlayerUI->GetOwner()->GetName());
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CorrectLocation(DeltaTime);

	if (GetCurrentState() == ECharacterState::EJump && GetActorLocation().Z < DeltaZ)
		IgnorePlatform(false);

	DeltaZ = GetActorLocation().Z;
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 게임플레이 키 바인딩
	check(PlayerInputComponent);
	// 이동 
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);
	// 점프
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::DoJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	// 공격
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &APlayerCharacter::Attack);
	PlayerInputComponent->BindAction("Attack", IE_Released, this, &APlayerCharacter::StopAttack);
	// 막기
	PlayerInputComponent->BindAction("SpecialAbility", IE_Pressed, this, &APlayerCharacter::CallSpecialAbility);
	PlayerInputComponent->BindAction("SpecialAbility", IE_Released, this, &APlayerCharacter::CallStopSpecialAbility);
	// 아래로 이동
	PlayerInputComponent->BindAction("MoveDown", IE_Pressed, this, &APlayerCharacter::SetIgnorePlatform);
	PlayerInputComponent->BindAction("MoveDown", IE_Released, this, &APlayerCharacter::SetBlockPlatform);
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

void APlayerCharacter::CorrectLocation(float DeltaTime)
{
	PlayerLocation = GetActorLocation();

	// 캐릭터의 X좌표 0으로 고정
	if (PlayerLocation.X != 0.f)
	{
		PlayerLocation.X = 0.f;
		SetActorLocation(PlayerLocation);
	}


	// 플레이어 위치 보정
	// 내 캐릭터가 아니면 위치 수신
	if (!IsMine)
	{
		UpdatedLocation = FMath::VInterpTo(PlayerLocation, NewLocation, DeltaTime, 10.f);

		////////// 테스트용 인풋
		//AddMovementInput(FVector(0.f, -1.f, 0.f), 1.f);
		if (CurrentState == ECharacterState::EIdle || CurrentState == ECharacterState::EJump)
		{
			//FRotator NewRotation;
			if (NewDir == 1)
			{
				SetActorRelativeRotation(FRotator(0.f, -90.f, 0.f));
			}
			else
			{
				SetActorRelativeRotation(FRotator(0.f, 90.f, 0.f));
			}
		}

		SetActorLocation(UpdatedLocation, false);
		//GEngine->AddOnScreenDebugMessage(2, 3.f, FColor::Blue, FString::Printf(TEXT("%f"), GetVelocity().Size()));
	}

	else
	{
		// 내 위치 송신

		SendDelay += 1;
		if (SendDelay == 3)
		{

			FMemory::Memcpy(body, &PlayerLocation.Y, sizeof(PlayerLocation.Y));
			FMemory::Memcpy(body + 4, &PlayerLocation.Z, sizeof(PlayerLocation.Z));
			FMemory::Memcpy(body + 8, &PlayerDir, sizeof(char));
			GameInstance->SendMessage(PACKET_TYPE::UPDATELOCATION, body, 9);
			SendDelay = 0;

			//GEngine->AddOnScreenDebugMessage(1, 3.f, FColor::White, FString::Printf(TEXT("%f"), GetVelocity().Size()));
		}

	}
}

void APlayerCharacter::UpdateLocation(FVector New, uint8 Dir)
{
	NewLocation = New;
	NewDir = Dir;
}

void APlayerCharacter::BeginOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	// 충돌한 엑터 모두 배열에 넣기
	TArray<AActor*> CollectedActors;
	GetCapsuleComponent()->GetOverlappingActors(CollectedActors);

	for (int32 iCollected = 0; iCollected < CollectedActors.Num(); ++iCollected)
	{
		// 충돌한 액터가 무기인경우 & 그 무기가 자신의 무기가 아닌 경우 & 무기가 활성화 되어있는 경우
		AWeapon* const OverlappedWeapon = Cast<AWeapon>(CollectedActors[iCollected]);
		if (OverlappedWeapon && (Weapon != OverlappedWeapon))
		{
			if (OverlappedWeapon->bIsActive)
			{
				// 피격 함수 호출
				AttackHit(OverlappedWeapon);
				OverlappedWeapon->bIsActive = false;
			}
			AWeapon_MagicWand* const OverlappedAbility = Cast<AWeapon_MagicWand>(OverlappedWeapon);
			if (OverlappedAbility && OverlappedAbility->bIsIncinerateOn)
			{
				AbilityHit(OverlappedAbility);
			}
		}
	}
	UE_LOG(LogTemp, Log, TEXT("%s 비긴 오버랩"), *OtherActor->GetName());
}

void APlayerCharacter::EndOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IsMine)
	{
		if (false == bIgnorePlatform)
			IgnorePlatform(false);
		UE_LOG(LogTemp, Log, TEXT("%s 앤드 오버랩"), *OtherActor->GetName());

	}
}

void APlayerCharacter::DoJump()
{
	if (IsMine)
	{
		if (CurrentState != ECharacterState::EIdle && CurrentState != ECharacterState::EJump)
			return;

		// 나 일경우, defenthit 애니메이션 전송
		anibody = (char)ECharacterAction::EA_Jump;
		GameInstance->SendMessage(PACKET_TYPE::UPDATESTATE, &anibody, 1);

		// 땅에 닿으면 점프 카운트 초기화
		// 애님 노티파이에서도 초기화 해주긴 하는데 혹시 몰라서
		if (GetCharacterMovement()->IsMovingOnGround() == true)
		{
			JumpCount = 0;
		}

		// 점프 카운트 2미만일시 점프 가능(최대 2단 점프)
		if (JumpCount < 2)
		{
			SetCurrentState(ECharacterState::EJump);
			LaunchCharacter(FVector(0.f, 0.f, 1.f) * GetCharacterMovement()->JumpZVelocity, false, true);
			JumpCount++;
			IgnorePlatform(true);
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
	if ((CurrentState == ECharacterState::EIdle) || (CurrentState == ECharacterState::EJump))
	{
		if (0 != val) {
			SetActorRelativeRotation(FRotator(0.f, -90.f * FMath::RoundFromZero(val), 0.f));
			if (val > 0)
				PlayerDir = 1;
			else if (val < 0)
				PlayerDir = 0;
		}

		// 입력받은 방향으로 이동
		AddMovementInput(FVector(0.f, -1.f, 0.f), val);
	}
}

void APlayerCharacter::AttackHit(AWeapon* OverlappedWeapon)
{
	//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, PlayerName + TEXT(" AttackHit()"));

	// 피격한 캐릭터의 전방 벡터
	FVector EnemyForwardVector = OverlappedWeapon->WeaponOwner->GetActorForwardVector();
	LastHitOwner = OverlappedWeapon->WeaponOwner->PlayerID;

	// 적과 나의 벡터 내적
	// 내적 값이 양수면 적이 내 뒤에 있음, 음수면 적이 내 앞에 있음
	float HitDot = FVector::DotProduct(GetActorForwardVector(), EnemyForwardVector);

	if (AnimInstance)
	{
		// 적이 내 앞에 있고 방어 중 일시 (방패 히트 애니메이션)
		if ((HitDot < 0) && CurrentState == ECharacterState::EDefend)
		{
			// 나 일경우, defenthit 애니메이션 전송
			if (IsMine)
			{
				//anibody[0] = (char)ECharacterAction::EA_DefendHit;
				//GameInstance->SendMessage(PACKET_TYPE::UPDATESTATE, anibody, 1);
			}

			AnimInstance->PlayDefendHit();
		}
		// 적이 내 뒤에 있거나 방어 중이 아닐시 (일반 히트 애니메이션)
		else
		{
			HitandKnockback(EnemyForwardVector, OverlappedWeapon->AttackDamage);
		}
	}
}

void APlayerCharacter::AbilityHit(AWeapon_MagicWand * OverlappedAbility)
{
	//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, PlayerName + TEXT(" AbiltityHit()"));
	LastHitOwner = OverlappedAbility->WeaponOwner->PlayerID;

	FVector EnemyForwardVector = OverlappedAbility->WeaponOwner->GetActorForwardVector();
	HitandKnockback(EnemyForwardVector, OverlappedAbility->IncinerateDamage);
}

void APlayerCharacter::HitandKnockback(FVector HitDirection, float HitDamage)
{
	//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, PlayerName + TEXT(" HitandKnockback()"));

	// 나 일경우 Hit 애니메이션 전송, hit damage 처리
	if (IsMine)
	{
		DamagePercent += HitDamage;
		GameInstance->SendMessage(PACKET_TYPE::UPDATEDMG, (char*)&DamagePercent, 4);
		StopAttack();
		CallStopSpecialAbility();
	}

	// 현재행동 중단하고 EHit 상태로 바꿔주기
	SetCurrentState(ECharacterState::EHit);
	AnimInstance->PlayGetHit();
	
	// 캐릭터 넉백
	// 넉백 로직 (맞은 방향 * 맞은데미지 * 캐릭터 데미지 퍼센트 * 2)
	float KnockBackValue = HitDamage * DamagePercent * 2.f;
	LaunchCharacter((HitDirection * KnockBackValue) + (FVector::UpVector * .5f * KnockBackValue), true, true);

	// 피격 이펙트 생성
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, GetActorTransform());
}

void APlayerCharacter::Attack()
{
	//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, PlayerName + TEXT(" Attack()"));

	// 나일경우 attack 애니메이션 전송
	if (IsMine)
	{
		if (CurrentState != ECharacterState::EIdle)
			return;

		anibody = (char)ECharacterAction::EA_Attack;
		GameInstance->SendMessage(PACKET_TYPE::UPDATESTATE, &anibody, 1);

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
	// 나 일경우 stopattack 신호 보냄.
	if (IsMine)
	{
		anibody = (char)ECharacterAction::EA_StopAttack;
		GameInstance->SendMessage(PACKET_TYPE::UPDATESTATE, &anibody, 1);
	}

	if (AnimInstance)
	{
		AnimInstance->bIsAttacking = false;
	}

}

void APlayerCharacter::CallSpecialAbility()
{
	if (IsMine)
	{
		anibody = (char)ECharacterAction::EA_SpecialAbility;
		GameInstance->SendMessage(PACKET_TYPE::UPDATESTATE, &anibody, 1);
	}
	SpecialAbility();
}

void APlayerCharacter::CallStopSpecialAbility()
{
	if (IsMine)
	{
		anibody = (char)ECharacterAction::EA_StopSpecialAbility;
		GameInstance->SendMessage(PACKET_TYPE::UPDATESTATE, &anibody, 1);
	}
	StopSpecialAbility();
}

void APlayerCharacter::SpecialAbility()
{
	UE_LOG(LogTemp, Warning, TEXT("This character has no ability!!!"));
}

void APlayerCharacter::StopSpecialAbility()
{
	// THERE IS NO WAY STOP SPECIAL ABILITY OF THIS CHARACTER!!!
}

void APlayerCharacter::Die()
{
	if (CurrentState == ECharacterState::EDead)
		return;

	// 사망 로그
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("플레이어 ") + PlayerName + TEXT(" 사망"), false);

	if (IsMine)
	{
		CurrentState = ECharacterState::EDead;
		PlayRingOutEffect();

		// 사망 패킷 전송
		anibody = (char)ECharacterAction::EA_Die;
		GameInstance->SendMessage(PACKET_TYPE::UPDATESTATE, &anibody, 1);

		// 나 죽인 사람에게 킬 패킷 전송
		char DieBody[4];
		FMemory::Memcpy(DieBody, &LastHitOwner, sizeof(uint32));
		GameInstance->SendMessage(PACKET_TYPE::KILL, DieBody, 4);
		
		// 사망 위젯으로 전환
		GameInstance->GameModeBase->ChangeWidget(GameInstance->GameModeBase->DieWidgetClass);

		if (GameInstance->PlayerList.Contains(PlayerID))
			GameInstance->PlayerList.Remove(PlayerID);

		Destroy();
	}
}

void APlayerCharacter::PlayRingOutEffect()
{
	UWorld* World = GetWorld();

	if (nullptr != World)
	{
		// 사망 이펙트
		World->SpawnActor<ARingOutExplosion>(ARingOutExplosion::StaticClass(), GetActorTransform());

		// 카메라 쉐이크
		if (nullptr != CameraShake)
		{
			World->GetFirstPlayerController()->PlayerCameraManager->PlayCameraShake(CameraShake);
		}
	}
}

void APlayerCharacter::SetIgnorePlatform()
{
	bIgnorePlatform = true;
	IgnorePlatform(true);
}

void APlayerCharacter::SetBlockPlatform()
{
	bIgnorePlatform = false;
}

void APlayerCharacter::IgnorePlatform(bool bIsIgnore)
{
	if (true == bIsIgnore)
	{
		if (IsMine)
		{
			anibody = (char)ECharacterAction::EA_IgnorePlatform;
			GameInstance->SendMessage(PACKET_TYPE::UPDATESTATE, &anibody, 1);
		}
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
	}
	else
	{
		if (IsMine)
		{
			anibody = (char)ECharacterAction::EA_BlockPlatform;
			GameInstance->SendMessage(PACKET_TYPE::UPDATESTATE, &anibody, 1);
		}
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Block);
	}
}
