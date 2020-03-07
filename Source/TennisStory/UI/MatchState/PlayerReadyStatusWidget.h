// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerReadyStatusWidget.generated.h"

class ATennisStoryPlayerState;

UCLASS()
class TENNISSTORY_API UPlayerReadyStatusWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	//Called before adding to viewport to get all current player states and bind to events
	UFUNCTION(BlueprintCallable, Category = "Player Widget")
	void SetUpWidget();
	
	//Called when removing from viewport to unbind events
	UFUNCTION(BlueprintCallable, Category = "Player Widget")
	void CleanUpWidget();

	UFUNCTION(BlueprintImplementableEvent, Category = "Player Widget")
	void AddPlayerStateWidget(ATennisStoryPlayerState* PlayerState);

	UFUNCTION(BlueprintImplementableEvent, Category = "Player Widget")
	void RemovePlayerStateWidget(ATennisStoryPlayerState* PlayerState);
};
