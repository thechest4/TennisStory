// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TennisStoryPlayerController.generated.h"

UCLASS()
class TENNISSTORY_API ATennisStoryPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Player")
	int GetPlayerNumber() const { return PlayerNumber; }

	void SetPlayerNumber(int AssignedNumber)
	{
		PlayerNumber = AssignedNumber;
	}

protected:
	UPROPERTY(Transient, Replicated)
	int PlayerNumber = -1;
};
