
#include "TennisStoryGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"
#include "Player/TennisStoryCharacter.h"
#include "Gameplay/HalfCourt.h"
#include "Gameplay/TennisBall.h"

//TODO(achester): get rid of this when we have a different solution for logging
#include "Engine.h"

ATennisStoryGameMode::ATennisStoryGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Player/TennisStoryCharacter_BP"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void ATennisStoryGameMode::StartPlay()
{
	if (!Courts.Num())
	{
		GetCourtsFromWorld();
	}

	if (DefaultBallClass)
	{
		FTransform BallSpawnTransform = FTransform::Identity;
		
		AHalfCourt* NearCourt = GetCourt(ECourtSide::NearCourt);
		if (NearCourt)
		{
			BallSpawnTransform = NearCourt->GetBallServiceTransform();
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		
		CurrentBallActor = GetWorld()->SpawnActor<ATennisBall>(DefaultBallClass, BallSpawnTransform, SpawnParams);
	}

	Super::StartPlay();
}

void ATennisStoryGameMode::RestartPlayer(AController* NewPlayer)
{
	if (NewPlayer == nullptr || NewPlayer->IsPendingKillPending())
	{
		GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("ATennisStoryGameMode::RestartPlayer - NewPlayer was null or pending kill"));
		return;
	}

	if (!GetDefaultPawnClassForController(NewPlayer))
	{
		GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("ATennisStoryGameMode::RestartPlayer - Default Pawn Class was null"));
		return;
	}

	TWeakObjectPtr<AHalfCourt> SpawnCourt = FindPlayerCourt(NewPlayer);
	if (SpawnCourt.IsValid())
	{
		FTransform SpawnTransform = SpawnCourt->GetPlayerServiceTransform();
		NewPlayer->SetPawn(SpawnDefaultPawnAtTransform(NewPlayer, SpawnTransform));

		if (!NewPlayer->GetPawn())
		{
			NewPlayer->FailedToSpawnPawn();
		}

		FinishRestartPlayer(NewPlayer, SpawnTransform.GetRotation().Rotator());
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("ATennisStoryGameMode::RestartPlayer - Could not find spawn court!"));
		Super::RestartPlayer(NewPlayer);
	}
}

TWeakObjectPtr<AHalfCourt> ATennisStoryGameMode::FindPlayerCourt(AController* NewPlayer)
{
	if (!Courts.Num())
	{
		GetCourtsFromWorld();
	}

	APlayerController* PlayerController = Cast<APlayerController>(NewPlayer);
	if (PlayerController)
	{
		for (TWeakObjectPtr<AHalfCourt> Court : Courts)
		{
			if (Court.IsValid() && static_cast<int>(Court->GetCourtSide()) == PlayerController->NetPlayerIndex)
			{
				return Court;
			}
		}
	}

	return nullptr;
}

void ATennisStoryGameMode::GetCourtsFromWorld()
{
	for (TActorIterator<AHalfCourt> It(GetWorld()); It; ++It)
	{
		AHalfCourt* Court = *It;
		Courts.Add(Court);
	}
}
