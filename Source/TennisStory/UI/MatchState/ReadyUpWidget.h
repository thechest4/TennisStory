// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReadyUpWidget.generated.h"

class ATennisStoryPlayerController;
class ATennisStoryPlayerState;

UCLASS()
class TENNISSTORY_API UReadyUpWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetUpWidget();

	void CleanUpWidget();

	ATennisStoryPlayerController* TryGetLocalPlayerController();

	UFUNCTION(BlueprintCallable)
	void TryGetLocalPlayerState();

	UFUNCTION(BlueprintCallable)
	void ToggleReadyState();

	UFUNCTION(BlueprintImplementableEvent)
	void HandleReadyStateUpdated(ATennisStoryPlayerState* PlayerState);
	
protected:
	
	UPROPERTY()
	ATennisStoryPlayerController* LocalPlayerController;

	UPROPERTY(BlueprintReadOnly)
	ATennisStoryPlayerState* LocalPlayerState;
};
