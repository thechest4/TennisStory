// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TennisStoryGameState.generated.h"

class ATennisBall;

UCLASS()
class TENNISSTORY_API ATennisStoryGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	TWeakObjectPtr<ATennisBall> GetTennisBall() const { return CurrentBallActor; }

protected:
	UPROPERTY(Transient, Replicated, BlueprintReadOnly)
	TWeakObjectPtr<ATennisBall> CurrentBallActor;

	friend class ATennisStoryGameMode;
};
