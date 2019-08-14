// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BigNantoGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class BIGNANTO_API ABigNantoGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	// 현재 위젯
	UPROPERTY()
	UUserWidget* CurrentWidget;

	// 게임 인스턴스 레퍼런스
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UBigNantoGameInstance* GameInstance;

public:
	ABigNantoGameModeBase();

	// 로그인 위젯
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UUserWidget> LoginWidgetClass;
	
	// 캐릭터 생성 위젯
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UUserWidget> CharacterMakeWidgetClass;

	// 사망시 위젯
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UUserWidget> DieWidgetClass;

	// 인게임 UI
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UUserWidget> InGameWidgetClass;

	// 위젯 전환 함수
	UFUNCTION(BlueprintCallable)
	void ChangeWidget(TSubclassOf<UUserWidget> NewWidegtClass);

	// 모든 위젯 삭제
	UFUNCTION(BlueprintCallable)
	void RemoveAllWidget();

};
