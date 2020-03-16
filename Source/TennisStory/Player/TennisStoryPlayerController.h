// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TennisStoryPlayerController.generated.h"

class UGameMenuWidget;

UCLASS()
class TENNISSTORY_API ATennisStoryPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void HideGameMenu();

protected:
	virtual void OnPossess(APawn* aPawn) override;

	virtual void SetupInputComponent() override;

	UPROPERTY(EditDefaultsOnly, Category = "Game Menu")
	TSubclassOf<UGameMenuWidget> GameMenuClass;

	UPROPERTY()
	UGameMenuWidget* GameMenuObject;

	void ShowGameMenu();
};
