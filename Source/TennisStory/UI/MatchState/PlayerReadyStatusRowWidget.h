// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerReadyStatusRowWidget.generated.h"

class ATennisStoryPlayerState;

UCLASS()
class TENNISSTORY_API UPlayerReadyStatusRowWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Player Widget")
	void SetUpRow(ATennisStoryPlayerState* PlayerState);
	
	UFUNCTION(BlueprintCallable, Category = "Player Widget")
	void CleanUpRow();

	UFUNCTION(BlueprintCallable, Category = "Player Widget")
	bool IsAssociatedWithPlayerState(ATennisStoryPlayerState* PlayerState);

	UFUNCTION(BlueprintImplementableEvent, Category = "Player Widget")
	void HandlePlayerReadyStateUpdated(ATennisStoryPlayerState* PlayerState);

protected:
	UPROPERTY(BlueprintReadOnly)
	ATennisStoryPlayerState* AssociatedPlayerState;
};
