// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReadyUpWidget.generated.h"

class ATennisStoryPlayerState;

UCLASS()
class TENNISSTORY_API UReadyUpWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetUpWidget();

	void CleanUpWidget();

	UFUNCTION(BlueprintCallable)
	void ToggleReadyState();

	UFUNCTION(BlueprintNativeEvent)
	void HandleReadyStateUpdated(ATennisStoryPlayerState* PlayerState);
	
	UFUNCTION(BlueprintImplementableEvent)
	void DisableWidgetInteraction();
	
	UFUNCTION(BlueprintImplementableEvent)
	void EnableWidgetInteraction();

protected:
	UPROPERTY(BlueprintReadOnly)
	ATennisStoryPlayerState* LocalPlayerState;
};
