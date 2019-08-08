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

	// �α��� ���� Ŭ���� ����
	static ConstructorHelpers::FClassFinder<UUserWidget> LoginWidget(TEXT("/Game/UMG/LoginUI"));
	if (LoginWidget.Succeeded())
		LoginWidgetClass = LoginWidget.Class;

	// ĳ���� ���� ���� Ŭ���� ����
	static ConstructorHelpers::FClassFinder<UUserWidget> CharacterMakeWidget(TEXT("/Game/UMG/CharacterMakeUI"));
	if (CharacterMakeWidget.Succeeded())
		CharacterMakeWidgetClass = CharacterMakeWidget.Class;

	// ĳ���� ���� ���� Ŭ���� ����
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
	
	// ���۽� �α��� �������� ��ȯ
	ChangeWidget(LoginWidgetClass);
}

void ABigNantoGameModeBase::ChangeWidget(TSubclassOf<UUserWidget> NewWidegtClass)
{
	// ���� �����ִ� ������ �ִٸ� �ٲٱ� ���� ������ ����
	if (CurrentWidget != nullptr)
	{
		CurrentWidget->RemoveFromViewport();
		CurrentWidget = nullptr;
	}

	// �ٲ� ������ ����Ʈ�� �����
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
	// ��� ���� ����
	if (CurrentWidget != nullptr)
	{
		CurrentWidget->RemoveFromViewport();
		CurrentWidget = nullptr;

		if (GameInstance->PlayerController)
			GameInstance->PlayerController->OffGameUI();
	}
}
