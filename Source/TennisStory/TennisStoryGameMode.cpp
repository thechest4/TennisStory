
#include "TennisStoryGameMode.h"
#include "TennisStoryGameState.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"
#include "Player/TennisStoryCharacter.h"
#include "Player/TennisStoryPlayerController.h"
#include "Gameplay/HalfCourt.h"
#include "Gameplay/Ball/TennisBall.h"
#include "Camera/CameraActor.h"
#include "Camera/CamPositioningComponent.h"

ATennisStoryGameMode::ATennisStoryGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Player/TennisStoryCharacter_BP"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	GameStateClass = ATennisStoryGameState::StaticClass();
	PlayerControllerClass = ATennisStoryPlayerController::StaticClass();

	AllowedBounces = 1;
}

void ATennisStoryGameMode::InitGameState()
{
	Super::InitGameState();

	TSGameState = Cast<ATennisStoryGameState>(GameState);

	checkf(TSGameState, TEXT("ATennisStoryGameMode::InitGameState - GameState was not derived from ATennisStoryGameState!"));

	static const int NumTeams = 2;
	for (int i = 0; i < NumTeams; i++)
	{
		FTeamData NewTeamData = FTeamData(i);
		TSGameState->TeamData.Add(NewTeamData);
	}

	TSGameState->InitScores(NumTeams);
	
	if (!TSGameState->Courts.Num())
	{
		GetCourtsFromWorld();
	}
}

void ATennisStoryGameMode::StartPlay()
{
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

		TSGameState->CurrentBallActor->OnBallOutOfBounds().AddUObject(this, &ATennisStoryGameMode::HandleBallOutOfBounds);
		TSGameState->CurrentBallActor->OnBallHitBounceLimit().AddUObject(this, &ATennisStoryGameMode::HandleBallHitBounceLimit);
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

	TWeakObjectPtr<AHalfCourt> SpawnCourt = nullptr;
	for (FTeamData& Team : TSGameState->TeamData)
	{
		if (!Team.AssignedPlayers.Num())
		{
			Team.AssignedPlayers.Add(TSPC);
			SpawnCourt = Team.AssignedCourt;
			break;
		}
	}

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

		SetPlayerDefaults(NewPlayer->GetPawn());

		K2_OnRestartPlayer(NewPlayer);
	}
}

void ATennisStoryGameMode::TeleportCharacterToCourt(ATennisStoryCharacter* Character)
{
	ATennisStoryPlayerController* PlayerController = Cast<ATennisStoryPlayerController>(Character->GetController());
	if (PlayerController)
	{
		const FTeamData& PlayerTeam = TSGameState->GetTeamForPlayer(PlayerController);
		if (PlayerTeam.TeamId >= 0 && PlayerTeam.AssignedCourt.IsValid())
		{
			Character->SetActorTransform(PlayerTeam.AssignedCourt->GetPlayerServiceTransform());
		}
	}
}

void ATennisStoryGameMode::TeleportBallToCourt()
{
	if (TSGameState->CurrentBallActor.IsValid())
	{
		FTransform CourtBallTransform = TSGameState->TeamData[0].AssignedCourt->GetBallServiceTransform();
		TSGameState->CurrentBallActor->SetActorLocation(CourtBallTransform.GetLocation());
		TSGameState->CurrentBallActor->SetActorRotation(CourtBallTransform.GetRotation());
	}
}

void ATennisStoryGameMode::GetCourtsFromWorld()
{
	for (TActorIterator<AHalfCourt> It(GetWorld()); It; ++It)
	{
		AHalfCourt* Court = *It;
		TSGameState->Courts.Add(Court);
		TSGameState->TeamData[static_cast<int>(Court->GetCourtSide())].AssignedCourt = Court;
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

void ATennisStoryGameMode::HandleBallOutOfBounds()
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("ATennisStoryGameMode::HandleBallOutOfBounds"));

	ResolvePoint(false);
}

void ATennisStoryGameMode::HandleBallHitBounceLimit()
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("ATennisStoryGameMode::HandleBallHitBounceLimit"));
	
	ResolvePoint(true);
}

void ATennisStoryGameMode::ResolvePoint(bool bLastPlayerWon)
{
	TWeakObjectPtr<ATennisStoryCharacter> WinningCharacter = TSGameState->CurrentBallActor->LastPlayerToHit;
	ATennisStoryPlayerController* WinningController = (WinningCharacter.IsValid()) ? Cast<ATennisStoryPlayerController>(WinningCharacter->Controller) : nullptr;

	bool bResult = TSGameState->AwardPoint(WinningController, bLastPlayerWon);

	if (!bResult)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, FString::Printf(TEXT("ATennisStoryGameMode::ResolvePoint - Failed to award point to player!"), *WinningController->GetName()));
	}
}
