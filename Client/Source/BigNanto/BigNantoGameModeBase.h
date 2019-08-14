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

	// ���� ����
	UPROPERTY()
	UUserWidget* CurrentWidget;

	// ���� �ν��Ͻ� ���۷���
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UBigNantoGameInstance* GameInstance;

public:
	ABigNantoGameModeBase();

	// �α��� ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UUserWidget> LoginWidgetClass;
	
	// ĳ���� ���� ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UUserWidget> CharacterMakeWidgetClass;

	// ����� ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UUserWidget> DieWidgetClass;

	// �ΰ��� UI
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UUserWidget> InGameWidgetClass;

	// ���� ��ȯ �Լ�
	UFUNCTION(BlueprintCallable)
	void ChangeWidget(TSubclassOf<UUserWidget> NewWidegtClass);

	// ��� ���� ����
	UFUNCTION(BlueprintCallable)
	void RemoveAllWidget();

};
