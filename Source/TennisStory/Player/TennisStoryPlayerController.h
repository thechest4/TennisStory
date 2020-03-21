// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TennisStoryPlayerController.generated.h"

class UPlayerWidgetManager;

UCLASS()
class TENNISSTORY_API ATennisStoryPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void HideGameMenu();

	UPlayerWidgetManager* GetPlayerWidgetManager() { return WidgetManagerObject; }

protected:
	virtual void SetPlayer(UPlayer* InPlayer) override;

	virtual void OnPossess(APawn* aPawn) override;

	virtual void SetupInputComponent() override;

	UPROPERTY(EditDefaultsOnly, Category = "Widget Manager")
	TSubclassOf<UPlayerWidgetManager> WidgetManagerClass;

	UPROPERTY()
	UPlayerWidgetManager* WidgetManagerObject;

	void CreatePlayerWidgetManager();

	void ShowGameMenu();
};
