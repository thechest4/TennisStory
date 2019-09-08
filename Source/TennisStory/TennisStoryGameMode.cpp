// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "TennisStoryGameMode.h"
#include "Player/TennisStoryCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATennisStoryGameMode::ATennisStoryGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Player/TennisStoryCharacter_BP"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
