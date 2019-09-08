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

	class ATennisBall* const GetTennisBall() { return CurrentBallActor; }
	void SetCurrentTennisBall(ATennisBall* TennisBall) 
	{ 
		//TODO(achester): Do we handle here destroying the old ball?  We probably only ever need one tennis ball actor for a whole match
		CurrentBallActor = TennisBall;
	};

protected:
	UPROPERTY()
	ATennisBall* CurrentBallActor;
};



