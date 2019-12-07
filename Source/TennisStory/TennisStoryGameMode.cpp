
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
	CurrentPlayState = EPlayState::Waiting;
}

void ATennisStoryGameMode::InitGameState()
{
	Super::InitGameState();

	TSGameState = Cast<ATennisStoryGameState>(GameState);

	checkf(TSGameState, TEXT("ATennisStoryGameMode::InitGameState - GameState was not derived from ATennisStoryGameState!"));

	for (int i = 0; i < MaxTeamNumber; i++)
	{
		FTeamData NewTeamData = FTeamData(i);
		TSGameState->TeamData.Add(NewTeamData);
	}

	TSGameState->InitScores(MaxTeamNumber, NumSets);

	//This only will show the scoreboard on the host, other clients will show it when the NumSets value replicates
	TSGameState->AddScoreWidgetToViewport();
	
	if (!TSGameState->Courts.Num())
	{
		GetCourtsFromWorld();
	}

	if (!CameraPositioningComp.IsValid())
	{
		GetCamPositioningCompFromWorld();
	}

	for (int i = 0; i < TSGameState->Courts.Num(); i++)
	{
		CameraPositioningComp->AddTrackedActor(TSGameState->Courts[i].Get());
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
			BallSpawnTransform = NearCourt->GetBallServiceTransform(TSGameState->GetServiceSideForNextPoint());
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		TSGameState->CurrentBallActor = GetWorld()->SpawnActor<ATennisBall>(DefaultBallClass, BallSpawnTransform, SpawnParams);
		TSGameState->CurrentBallActor->SetBallState(ETennisBallState::ServiceState);

		TSGameState->CurrentBallActor->OnBallOutOfBounds().AddUObject(this, &ATennisStoryGameMode::HandleBallOutOfBounds);
		TSGameState->CurrentBallActor->OnBallHitBounceLimit().AddUObject(this, &ATennisStoryGameMode::HandleBallHitBounceLimit);
	}

	Super::StartPlay();
	
	//NOTE(achester): I believe this only works because the host player happens to spawn and be set up before StartPlay (and we only ever have the host serve first)
	TSGameState->CurrentServingCharacter = Cast<ATennisStoryCharacter>(TSGameState->TeamData[TSGameState->CurrentServiceTeam].AssignedPlayers[0]->GetPawn());

	SetUpNextPoint();
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
		FTransform SpawnTransform = SpawnCourt->GetPlayerServiceTransform(EServiceSide::Deuce);

		APawn* NewPawn = SpawnDefaultPawnAtTransform(NewPlayer, SpawnTransform);
		NewPlayer->SetPawn(NewPawn);

		if (!NewPlayer->GetPawn())
		{
			NewPlayer->FailedToSpawnPawn();
		}

		ATennisStoryCharacter* TennisChar = Cast<ATennisStoryCharacter>(NewPawn);
		if (TennisChar)
		{
			TennisChar->ServerDesiredRotation = SpawnTransform.GetRotation();

			AllCharacters.Add(TennisChar);

			TennisChar->CacheCourtAimVector(SpawnCourt->GetActorForwardVector());

			if (!CameraPositioningComp.IsValid())
			{
				GetCamPositioningCompFromWorld();
			}

			CameraPositioningComp->AddTrackedActor(TennisChar);

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
			if (PlayerTeam.TeamId == TSGameState->CurrentServiceTeam)
			{
				Character->Multicast_SetActorTransform(PlayerTeam.AssignedCourt->GetPlayerServiceTransform(TSGameState->GetServiceSideForNextPoint()));
				Character->ServerDesiredRotation = PlayerTeam.AssignedCourt->GetPlayerServiceTransform(TSGameState->GetServiceSideForNextPoint()).GetRotation();
			}
			else
			{
				Character->Multicast_SetActorTransform(PlayerTeam.AssignedCourt->GetReturnerTransform(TSGameState->GetServiceSideForNextPoint()));
				Character->ServerDesiredRotation = PlayerTeam.AssignedCourt->GetReturnerTransform(TSGameState->GetServiceSideForNextPoint()).GetRotation();
			}
		}
	}
}

void ATennisStoryGameMode::TeleportBallToCourt()
{
	if (TSGameState->CurrentBallActor.IsValid())
	{
		FTransform CourtBallTransform = TSGameState->TeamData[TSGameState->CurrentServiceTeam].AssignedCourt->GetBallServiceTransform(TSGameState->GetServiceSideForNextPoint());
		TSGameState->CurrentBallActor->SetActorLocation(CourtBallTransform.GetLocation());
		TSGameState->CurrentBallActor->SetActorRotation(CourtBallTransform.GetRotation());
	}
}

void ATennisStoryGameMode::SetUpNextPoint()
{
	for (int i = 0; i < AllCharacters.Num(); i++)
	{
		if (AllCharacters[i].IsValid())
		{
			TeleportCharacterToCourt(AllCharacters[i].Get());
		}
	}
	
	if (TSGameState->CurrentServingCharacter.IsValid())
	{
		TSGameState->CurrentBallActor->AttachToComponent(TSGameState->CurrentServingCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("hand_l"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("ATennisStoryGameMode::SetUpNextPoint - CurrentServingCharacter was not valid!"));
	}

	TSGameState->CurrentBallActor->SetBallState(ETennisBallState::ServiceState);

	CurrentPlayState = EPlayState::PlayingPoint;
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
	ResolvePoint(false);
}

void ATennisStoryGameMode::HandleBallHitBounceLimit()
{
	ResolvePoint(true);
}

void ATennisStoryGameMode::ResolvePoint(bool bLastPlayerWon)
{
	if (CurrentPlayState != EPlayState::PlayingPoint)
	{
		return;
	}

	TWeakObjectPtr<ATennisStoryCharacter> LastPlayerToHit = TSGameState->CurrentBallActor->LastPlayerToHit;
	ATennisStoryPlayerController* PlayerController = (LastPlayerToHit.IsValid()) ? Cast<ATennisStoryPlayerController>(LastPlayerToHit->Controller) : nullptr;

	if (!LastPlayerToHit.IsValid())
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("ATennisStoryGameMode::ResolvePoint - LastPlayerToHit was null"));
		return;
	}
	
	int WinnerTeamId = TSGameState->GetTeamIdForPlayer(PlayerController);

	if (!bLastPlayerWon)
	{
		//NOTE(achester): here's where we make the assumption that there will only be 2 teams, for simplicity's sake
		WinnerTeamId = (WinnerTeamId) ? 0 : 1;
	}

	if (WinnerTeamId < 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("ATennisStoryGameMode::ResolvePoint - Failed to get valid team id for player"));
		return;
	}

	TSGameState->AwardPoint(WinnerTeamId);

	int LoserTeamId = (WinnerTeamId) ? 0 : 1;
	int WinnerGameScore = TSGameState->CurrentGameScore.Scores[WinnerTeamId];
	int LoserGameScore = TSGameState->CurrentGameScore.Scores[LoserTeamId];

	if (WinnerGameScore >= PointsToWinGame && WinnerGameScore - LoserGameScore >= MarginToWinGame)
	{
		TSGameState->AwardGame(WinnerTeamId);
		
		int WinnerSetScore = TSGameState->CurrentMatchScores[WinnerTeamId].SetScores[TSGameState->CurrentSet];
		int LoserSetScore = TSGameState->CurrentMatchScores[LoserTeamId].SetScores[TSGameState->CurrentSet];

		if (WinnerSetScore >= GamesToWinSet && WinnerSetScore - LoserSetScore >= MarginToWinSet)
		{
			TSGameState->CurrentMatchScores[WinnerTeamId].SetsWon++;
			
			int WinnerSets = TSGameState->CurrentMatchScores[WinnerTeamId].SetsWon;
			int LoserSets = TSGameState->CurrentMatchScores[LoserTeamId].SetsWon;

			float TotalSets = NumSets;
			int SetsToWinMatch = FMath::CeilToInt(TotalSets / 2.f);

			if (WinnerSets >= SetsToWinMatch)
			{
				TSGameState->InitScores(MaxTeamNumber, NumSets);
			}
			else
			{
				TSGameState->CurrentSet++;
			}
		}
		
		TSGameState->CurrentServiceTeam = (TSGameState->CurrentServiceTeam) ? 0 : 1;
		TSGameState->CurrentServingCharacter = Cast<ATennisStoryCharacter>(TSGameState->TeamData[TSGameState->CurrentServiceTeam].AssignedPlayers[0]->GetPawn());

		if (TSGameState->GetTotalGameCountForCurrentSet() % 2)
		{
			SwitchSides();
		}
	}

	CurrentPlayState = EPlayState::Waiting;

	FTimerHandle NextPointHandle;
	GetWorldTimerManager().SetTimer(NextPointHandle, this, &ATennisStoryGameMode::SetUpNextPoint, 1.5f);
}

void ATennisStoryGameMode::SwitchSides()
{
	for (int i = 0; i < TSGameState->TeamData.Num(); i++)
	{
		FTeamData& CurrentTeam = TSGameState->TeamData[i];
		int NewCourt = (static_cast<int>(CurrentTeam.AssignedCourt->GetCourtSide())) ? 0 : 1;
		CurrentTeam.AssignedCourt = TSGameState->GetCourt(static_cast<ECourtSide>(NewCourt));

		for (int j = 0; j < CurrentTeam.AssignedPlayers.Num(); j++)
		{
			ATennisStoryCharacter* Character = Cast<ATennisStoryCharacter>(CurrentTeam.AssignedPlayers[j]->GetPawn());
			Character->CacheCourtAimVector(CurrentTeam.AssignedCourt->GetActorForwardVector());
		}
	}
}
