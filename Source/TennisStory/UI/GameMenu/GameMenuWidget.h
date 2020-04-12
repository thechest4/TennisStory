// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameMenuWidget.generated.h"

DECLARE_DELEGATE(FGameMenuWantsToCloseEvent)
DECLARE_DELEGATE(FReturningToMainMenuEvent)

UCLASS()
class TENNISSTORY_API UGameMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FGameMenuWantsToCloseEvent& OnGameMenuWantsClose() { return GameMenuWantsCloseEvent; }

	FReturningToMainMenuEvent& OnReturningToMainMenu() { return ReturningToMainMenuEvent; }

	UFUNCTION(BlueprintCallable, Category = "Game Menu")
	void BroadcastWantsCloseEvent();

	UFUNCTION(BlueprintCallable, Category = "Game Menu")
	void BroadcastReturningToMainMenuEvent();

private:
	FGameMenuWantsToCloseEvent GameMenuWantsCloseEvent;
	
	FReturningToMainMenuEvent ReturningToMainMenuEvent;
};
