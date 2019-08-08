// Fill out your copyright notice in the Description page of Project Settings.


#include "BigNantoGameModeBase.h"
#include "BigNantoGameInstance.h"
#include "Blueprint/UserWidget.h"
#include "BigNantoPlayerController.h"
#include "PlayerCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "CenterViewPawn.h"

ABigNantoGameModeBase::ABigNantoGameModeBase()
{
	DefaultPawnClass = ACenterViewPawn::StaticClass();
	PlayerControllerClass = ABigNantoPlayerController::StaticClass();

	// 로그인 위젯 클래스 연결
	static ConstructorHelpers::FClassFinder<UUserWidget> LoginWidget(TEXT("/Game/UMG/LoginUI"));
	if (LoginWidget.Succeeded())
		LoginWidgetClass = LoginWidget.Class;

	// 캐릭터 생성 위젯 클래스 연결
	static ConstructorHelpers::FClassFinder<UUserWidget> CharacterMakeWidget(TEXT("/Game/UMG/CharacterMakeUI"));
	if (CharacterMakeWidget.Succeeded())
		CharacterMakeWidgetClass = CharacterMakeWidget.Class;

	// 캐릭터 생성 위젯 클래스 연결
	static ConstructorHelpers::FClassFinder<UUserWidget> DieWidget(TEXT("/Game/UMG/DieUI"));
	if (DieWidget.Succeeded())
		DieWidgetClass = DieWidget.Class;
}

void ABigNantoGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	GameInstance = Cast<UBigNantoGameInstance>(GetGameInstance());
	if (nullptr == GameInstance)
		return;

	GameInstance->GameModeBase = this;
	
	// 시작시 로그인 위젯으로 전환
	ChangeWidget(LoginWidgetClass);
}

void ABigNantoGameModeBase::ChangeWidget(TSubclassOf<UUserWidget> NewWidegtClass)
{
	// 현재 켜져있는 위젯이 있다면 바꾸기 위해 위젯을 꺼줌
	if (CurrentWidget != nullptr)
	{
		CurrentWidget->RemoveFromViewport();
		CurrentWidget = nullptr;
	}

	// 바꿀 위젯을 뷰포트에 깔아줌
	if (NewWidegtClass != nullptr)
	{
		CurrentWidget = CreateWidget(GetWorld(), NewWidegtClass);
		if (CurrentWidget != nullptr)
		{
			CurrentWidget->AddToViewport();
			if (GameInstance->PlayerController)
				GameInstance->PlayerController->OnGameUI();
		}
	}
}

void ABigNantoGameModeBase::RemoveAllWidget()
{
	// 모든 위젯 삭제
	if (CurrentWidget != nullptr)
	{
		CurrentWidget->RemoveFromViewport();
		CurrentWidget = nullptr;

		if (GameInstance->PlayerController)
			GameInstance->PlayerController->OffGameUI();
	}
}
