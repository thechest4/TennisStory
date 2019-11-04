
#include "TennisStoryGameMode.h"
#include "TennisStoryGameState.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"
#include "Player/TennisStoryCharacter.h"
#include "Player/TennisStoryPlayerController.h"
#include "Gameplay/HalfCourt.h"
#include "Gameplay/TennisBall.h"
#include "Camera/CameraActor.h"
#include "Camera/CamPositioningComponent.h"

//TODO(achester): get rid of this when we have a different solution for logging
#include "Engine.h"

ATennisStoryGameMode::ATennisStoryGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Player/TennisStoryCharacter_BP"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	GameStateClass = ATennisStoryGameState::StaticClass();
	PlayerControllerClass = ATennisStoryPlayerController::StaticClass();
}

void ATennisStoryGameMode::InitGameState()
{
	Super::InitGameState();

	TSGameState = Cast<ATennisStoryGameState>(GameState);

	checkf(TSGameState, TEXT("ATennisStoryGameMode::InitGameState - GameState was not derived from ATennisStoryGameState!"));
}

void ATennisStoryGameMode::StartPlay()
{
	if (!CameraPositioningComp.IsValid())
	{
		GetCamPositioningCompFromWorld();
	}

	if (!TSGameState->Courts.Num())
	{
		GetCourtsFromWorld();
	}

	if (DefaultBallClass)
	{
		FTransform BallSpawnTransform = FTransform::Identity;

		AHalfCourt* NearCourt = TSGameState->GetCourt(ECourtSide::NearCourt);
		if (NearCourt)
		{
			BallSpawnTransform = NearCourt->GetBallServiceTransform();
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		TSGameState->CurrentBallActor = GetWorld()->SpawnActor<ATennisBall>(DefaultBallClass, BallSpawnTransform, SpawnParams);
		TSGameState->CurrentBallActor->SetBallState(ETennisBallState::ServiceState);

		if (CameraPositioningComp.IsValid())
		{
			CameraPositioningComp->AddTrackedActor(Cast<AActor>(TSGameState->CurrentBallActor));
		}
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

	ATennisStoryPlayerController* TSPC = Cast<ATennisStoryPlayerController>(NewPlayer);

	checkf(TSPC, TEXT("ATennisStoryGameMode::PostLogin - PlayerController was not derived from ATennisStoryPlayerController!"));

	TSPC->SetPlayerNumber(NextPlayerNumberToAssign);
	NextPlayerNumberToAssign++;

	TWeakObjectPtr<AHalfCourt> SpawnCourt = FindPlayerCourt(NewPlayer);
	if (SpawnCourt.IsValid())
	{
		FTransform SpawnTransform = SpawnCourt->GetPlayerServiceTransform();

		APawn* NewPawn = SpawnDefaultPawnAtTransform(NewPlayer, SpawnTransform);
		NewPlayer->SetPawn(NewPawn);

		if (!NewPlayer->GetPawn())
		{
			NewPlayer->FailedToSpawnPawn();
		}

		ATennisStoryCharacter* TennisChar = Cast<ATennisStoryCharacter>(NewPawn);
		if (TennisChar)
		{
			TennisChar->CacheCourtAimVector(SpawnCourt->GetActorForwardVector());

			if (!CameraPositioningComp.IsValid())
			{
				GetCamPositioningCompFromWorld();
			}

			if (CameraPositioningComp.IsValid())
			{
				CameraPositioningComp->AddTrackedActor(Cast<AActor>(TennisChar));
			}

			APlayerController* PlayerController = Cast<APlayerController>(NewPlayer);
			if (PlayerController && CameraPositioningComp.IsValid())
			{
				PlayerController->SetViewTarget(CameraPositioningComp->GetOwner());
			}
		}

		FinishRestartPlayer(NewPlayer, SpawnTransform.GetRotation().Rotator());
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("ATennisStoryGameMode::RestartPlayer - Could not find spawn court!"));
		Super::RestartPlayer(NewPlayer);
	}
}

void ATennisStoryGameMode::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	NewPlayer->Possess(NewPlayer->GetPawn());

	if (NewPlayer->GetPawn() == nullptr)
	{
		NewPlayer->FailedToSpawnPawn();
	}
	else
	{
		NewPlayer->ClientSetRotation(NewPlayer->GetPawn()->GetActorRotation(), true);
		NewPlayer->SetControlRotation(FRotator::ZeroRotator);

		SetPlayerDefaults(NewPlayer->GetPawn());

		K2_OnRestartPlayer(NewPlayer);
	}
}

TWeakObjectPtr<AHalfCourt> ATennisStoryGameMode::FindPlayerCourt(AController* NewPlayer)
{
	if (!TSGameState->Courts.Num())
	{
		GetCourtsFromWorld();
	}

	ATennisStoryPlayerController* PlayerController = Cast<ATennisStoryPlayerController>(NewPlayer);
	if (PlayerController)
	{
		for (TWeakObjectPtr<AHalfCourt> Court : TSGameState->Courts)
		{
			if (Court.IsValid() && static_cast<int>(Court->GetCourtSide()) == PlayerController->GetPlayerNumber())
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
		TSGameState->Courts.Add(Court);
	}
}

void ATennisStoryGameMode::GetCamPositioningCompFromWorld()
{
	//Find all CameraActors and cache a pointer to the one with a CamPositioningComponent
	for (TActorIterator<ACameraActor> It(GetWorld()); It; ++It)
	{
		ACameraActor* CamActor = *It;

		UActorComponent* ActorComp = CamActor->GetComponentByClass(UCamPositioningComponent::StaticClass());
		if (ActorComp)
		{
			CameraPositioningComp = Cast<UCamPositioningComponent>(ActorComp);
			return;
		}
	}

	GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("ATennisStoryGameMode::GetCamPositioningCompFromWorld - Could not find Camera Positioning Component!"));
}
