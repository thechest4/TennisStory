// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameMenuWidget.generated.h"

DECLARE_DELEGATE(FGameMenuWantsToCloseEvent)

UCLASS()
class TENNISSTORY_API UGameMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FGameMenuWantsToCloseEvent& OnGameMenuWantsClose() { return GameMenuWantsCloseEvent; }

	UFUNCTION(BlueprintCallable, Category = "Game Menu")
	void BroadcastWantsCloseEvent();

private:
	FGameMenuWantsToCloseEvent GameMenuWantsCloseEvent;
	
};
