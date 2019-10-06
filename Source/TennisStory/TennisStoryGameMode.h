// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TennisStoryGameMode.generated.h"

UCLASS(minimalapi)
class ATennisStoryGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATennisStoryGameMode();

	TWeakObjectPtr<class ATennisBall> GetTennisBall() { return CurrentBallActor; }
	void SetCurrentTennisBall(TWeakObjectPtr<ATennisBall> TennisBall) 
	{ 
		//TODO(achester): Do we handle here destroying the old ball?  We probably only ever need one tennis ball actor for a whole match
		CurrentBallActor = TennisBall;
	};

	void AddCourt(TWeakObjectPtr<class AHalfCourt> HalfCourt)
	{
		Courts.Add(HalfCourt);
	}

	TArray<TWeakObjectPtr<AHalfCourt>> GetAllCourts()
	{
		return Courts;
	}

protected:
	UPROPERTY()
	TWeakObjectPtr<ATennisBall> CurrentBallActor;

	UPROPERTY()
	TArray<TWeakObjectPtr<AHalfCourt>> Courts;
};



