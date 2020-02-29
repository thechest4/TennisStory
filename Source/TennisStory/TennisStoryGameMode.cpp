
#include "TennisStoryGameMode.h"
#include "TennisStoryGameState.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"
#include "Player/TennisStoryCharacter.h"
#include "Player/TennisStoryPlayerController.h"
#include "Gameplay/HalfCourt.h"
#include "Gameplay/BounceLocationMarker.h"
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
	AllowedFaults = 1;
}

void ATennisStoryGameMode::InitGameState()
{
	Super::InitGameState();

	TSGameState = Cast<ATennisStoryGameState>(GameState);

	checkf(TSGameState, TEXT("ATennisStoryGameMode::InitGameState - GameState was not derived from ATennisStoryGameState!"));
	
	TSGameState->CurrentPlayState = EPlayState::Waiting;

	for (int i = 0; i < MaxTeamNumber; i++)
	{
		FTeamData NewTeamData = FTeamData(i);

		if (i < TSGameState->TeamNames.Num())
		{
			NewTeamData.TeamName = TSGameState->TeamNames[i];
		}
		
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

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		TSGameState->CurrentBallActor = GetWorld()->SpawnActor<ATennisBall>(DefaultBallClass, BallSpawnTransform, SpawnParams);
		TSGameState->CurrentBallActor->SetBallState(ETennisBallState::ServiceState);

		TSGameState->CurrentBallActor->OnBallOutOfBounds().AddUObject(this, &ATennisStoryGameMode::HandleBallOutOfBounds);
		TSGameState->CurrentBallActor->OnBallHitBounceLimit().AddUObject(this, &ATennisStoryGameMode::HandleBallHitBounceLimit);
	}

	if (DefaultBounceMarkerClass)
	{
		FTransform SpawnTransform = FTransform::Identity;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		BounceMarkerActor = GetWorld()->SpawnActor<ABounceLocationMarker>(DefaultBounceMarkerClass, SpawnTransform, SpawnParams);
	}

	Super::StartPlay();
	
	//NOTE(achester): I believe this only works because the host player happens to spawn and be set up before StartPlay (and we only ever have the host serve first)
	TSGameState->CurrentServingCharacter = TSGameState->TeamData[TSGameState->CurrentServiceTeam].AssignedCharacters[0];

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

	checkf(TSPC, TEXT("ATennisStoryGameMode::RestartPlayer - PlayerController was not derived from ATennisStoryPlayerController!"));

	TWeakObjectPtr<AHalfCourt> SpawnCourt = nullptr;
	FTeamData* PlayerTeam = nullptr;
	for (FTeamData& Team : TSGameState->TeamData)
	{
		//NOTE(achester): This check assumes that the appropriate team for the new player is the one that has no players currently.  Will need to be changed for doubles!
		if (!Team.AssignedPlayers.Num())
		{
			Team.AssignedPlayers.Add(TSPC);
			SpawnCourt = Team.AssignedCourt;
			PlayerTeam = &Team;
			break;
		}
	}
	
	checkf(PlayerTeam, TEXT("ATennisStoryGameMode::RestartPlayer - PlayerTeam pointer was left uninitialized!"));

	if (SpawnCourt.IsValid() && PlayerTeam)
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
			PlayerTeam->AssignedCharacters.Add(TennisChar);

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
	if (Character)
	{
		const FTeamData& PlayerTeam = TSGameState->GetTeamForCharacter(Character);
		if (PlayerTeam.TeamId >= 0 && PlayerTeam.AssignedCourt.IsValid())
		{
			if (PlayerTeam.TeamId == TSGameState->CurrentServiceTeam)
			{
				Character->Multicast_SetActorTransform(PlayerTeam.AssignedCourt->GetPlayerServiceTransform(TSGameState->GetServiceSide()));
				Character->ServerDesiredRotation = PlayerTeam.AssignedCourt->GetPlayerServiceTransform(TSGameState->GetServiceSide()).GetRotation();
			}
			else
			{
				Character->Multicast_SetActorTransform(PlayerTeam.AssignedCourt->GetReturnerTransform(TSGameState->GetServiceSide()));
				Character->ServerDesiredRotation = PlayerTeam.AssignedCourt->GetReturnerTransform(TSGameState->GetServiceSide()).GetRotation();
			}
		}
	}
}

void ATennisStoryGameMode::SetUpNextPoint()
{
	for (int i = 0; i < AllCharacters.Num(); i++)
	{
		if (AllCharacters[i].IsValid())
		{
			AllCharacters[i]->CancelAllAbilities();
			TeleportCharacterToCourt(AllCharacters[i].Get());
		}
	}
	
	TSGameState->CurrentBallActor->SetBallState(ETennisBallState::ServiceState);
	if (TSGameState->CurrentServingCharacter.IsValid())
	{
		TSGameState->CurrentServingCharacter->Multicast_EnterServiceState();		
		TSGameState->CurrentServingCharacter->AttachBallToPlayer(TSGameState->CurrentBallActor.Get());
	
		TWeakObjectPtr<AHalfCourt> Court = TSGameState->GetCourtForCharacter(TSGameState->CurrentServingCharacter.Get());

		checkf(Court.IsValid(), TEXT("ATennisStoryGameMode::SetUpNextPoint - Court was null"))

		FVector ClampLocation1, ClampLocation2;
		Court->GetServiceClampLocations(TSGameState->GetServiceSide(), ClampLocation1, ClampLocation2);
		TSGameState->CurrentServingCharacter->ClampLocation(ClampLocation1, ClampLocation2);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("ATennisStoryGameMode::SetUpNextPoint - CurrentServingCharacter was not valid!"));
	}

	TSGameState->CurrentPlayState = EPlayState::Service;
}

void ATennisStoryGameMode::ReportServeHit()
{
	TSGameState->CurrentPlayState = EPlayState::PlayingPoint;
}

void ATennisStoryGameMode::DetermineHitLegality(ATennisStoryCharacter* Character)
{
	if (TSGameState->CurrentBallActor->bWasLastHitAServe)
	{
		if (TSGameState->CurrentBallActor->GetCurrentNumBounces() == 0)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("ATennisStoryGameMode::DetermineHitLegality - Can't hit serve before first bounce"));
			ResolvePoint(true, false, FVector::ZeroVector, EPointResolutionType::IllegalHit);
		}
		else if (false)
		{
			//Handle last player not being the correct returner
		}
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

void ATennisStoryGameMode::HandleBallOutOfBounds(EBoundsContext BoundsContext, FVector BounceLocation)
{
	if (BoundsContext == EBoundsContext::ServiceAd || BoundsContext == EBoundsContext::ServiceDeuce)
	{
		if (TSGameState->CurrentFaultCount < AllowedFaults)
		{
			if (TSGameState->CurrentPlayState != EPlayState::PlayingPoint)
			{
				return;
			}

			TSGameState->CurrentFaultCount++;
			
			TSGameState->CurrentPlayState = EPlayState::Waiting;

			TSGameState->AddCalloutWidgetToViewport(1.5f, FText::FromString(TEXT("FAULT")), FText::FromString(TEXT("SECOND SERVE")));

			FTimerHandle NextPointHandle;
			GetWorldTimerManager().SetTimer(NextPointHandle, this, &ATennisStoryGameMode::SetUpNextPoint, 1.5f);
			
			if (BounceMarkerActor.IsValid())
			{
				BounceMarkerActor->Multicast_ShowMarkerAtLocation(BounceLocation, 0.75f);
			}
		}
		else
		{
			ResolvePoint(false, true, BounceLocation, EPointResolutionType::DoubleFault);
		}
	}
	else
	{
		ResolvePoint(false, true, BounceLocation, EPointResolutionType::Out);
	}
}

void ATennisStoryGameMode::HandleBallHitBounceLimit()
{
	ResolvePoint(true, false, FVector::ZeroVector, EPointResolutionType::Winner);
}

void ATennisStoryGameMode::ResolvePoint(bool bLastPlayerWon, bool bShowBounceLocation, FVector BounceLocation, EPointResolutionType PointType)
{
	EPointResolutionContext CurrentPointResolutionContext = EPointResolutionContext::Point;

	if (TSGameState->CurrentPlayState != EPlayState::PlayingPoint)
	{
		return;
	}
	
	if (bShowBounceLocation && BounceMarkerActor.IsValid())
	{
		BounceMarkerActor->Multicast_ShowMarkerAtLocation(BounceLocation, 1.5f);
	}

	TWeakObjectPtr<ATennisStoryCharacter> LastPlayerToHit = TSGameState->CurrentBallActor->LastPlayerToHit;
	if (!LastPlayerToHit.IsValid())
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("ATennisStoryGameMode::ResolvePoint - LastPlayerToHit was null"));
		return;
	}
	
	int WinnerTeamId = TSGameState->GetTeamIdForCharacter(LastPlayerToHit.Get());

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
	
	TSGameState->CurrentFaultCount = 0;
	TSGameState->AwardPoint(WinnerTeamId);

	int LoserTeamId = (WinnerTeamId) ? 0 : 1;
	int WinnerGameScore = TSGameState->CurrentGameScore.Scores[WinnerTeamId];
	int LoserGameScore = TSGameState->CurrentGameScore.Scores[LoserTeamId];

	if (WinnerGameScore >= PointsToWinGame && WinnerGameScore - LoserGameScore >= MarginToWinGame)
	{
		CurrentPointResolutionContext = EPointResolutionContext::Game;

		TSGameState->AwardGame(WinnerTeamId);
		
		int WinnerSetScore = TSGameState->CurrentMatchScores[WinnerTeamId].SetScores[TSGameState->CurrentSet];
		int LoserSetScore = TSGameState->CurrentMatchScores[LoserTeamId].SetScores[TSGameState->CurrentSet];

		if (WinnerSetScore >= GamesToWinSet && WinnerSetScore - LoserSetScore >= MarginToWinSet)
		{
			CurrentPointResolutionContext = EPointResolutionContext::Set;

			TSGameState->CurrentMatchScores[WinnerTeamId].SetsWon++;
			
			int WinnerSets = TSGameState->CurrentMatchScores[WinnerTeamId].SetsWon;
			int LoserSets = TSGameState->CurrentMatchScores[LoserTeamId].SetsWon;

			float TotalSets = NumSets;
			int SetsToWinMatch = FMath::CeilToInt(TotalSets / 2.f);

			if (WinnerSets >= SetsToWinMatch)
			{
				CurrentPointResolutionContext = EPointResolutionContext::Match;

				TSGameState->InitScores(MaxTeamNumber, NumSets);
			}
			else
			{
				TSGameState->CurrentSet++;
			}
		}
		
		TSGameState->CurrentServiceTeam = (TSGameState->CurrentServiceTeam) ? 0 : 1;
		TSGameState->CurrentServingCharacter = TSGameState->TeamData[TSGameState->CurrentServiceTeam].AssignedCharacters[0];

		if (TSGameState->GetTotalGameCountForCurrentSet() % 2)
		{
			SwitchSides();
		}
	}

	TSGameState->CurrentPlayState = EPlayState::Waiting;

	FTimerHandle NextPointHandle;
	GetWorldTimerManager().SetTimer(NextPointHandle, this, &ATennisStoryGameMode::SetUpNextPoint, 1.5f);

	FString ResolutionTypeString = FString();
	FString ScoreCalloutString = FString();

	TArray<FString> TeamNameArray = GenerateTeamNameArray();

	if (CurrentPointResolutionContext == EPointResolutionContext::Point)
	{
		switch (PointType)
		{
			default:
			{
				ResolutionTypeString = FString(TEXT("Undefined Point Type"));
				break;
			}
			case EPointResolutionType::Out:
			{
				ResolutionTypeString = FString(TEXT("OUT"));
				break;
			}
			case EPointResolutionType::Winner:
			{
				ResolutionTypeString = FString(TEXT("WINNER"));
				break;
			}
			case EPointResolutionType::DoubleFault:
			{
				ResolutionTypeString = FString(TEXT("DOUBLE FAULT"));
				break;
			}
			case EPointResolutionType::IllegalHit:
			{
				ResolutionTypeString = FString(TEXT("ILLEGAL HIT"));
				break;
			}
		}
	}

	switch (CurrentPointResolutionContext)
	{
		default:
		{
			ScoreCalloutString = FString(TEXT("Undefined Point Context"));
			break;
		}
		case EPointResolutionContext::Point:
		{
			ScoreCalloutString = TSGameState->CurrentGameScore.GetGameScoreDisplayString(TeamNameArray);
			break;
		}
		case EPointResolutionContext::Game:
		{
			//Current Set Score
			break;
		}
		case EPointResolutionContext::Set:
		{
			//Current Match Score
			break;
		}
		case EPointResolutionContext::Match:
		{
			//Probably not a needed case
			break;
		}
	}
	
	TSGameState->AddCalloutWidgetToViewport(1.5f, FText::FromString(ResolutionTypeString), FText::FromString(ScoreCalloutString));
}

void ATennisStoryGameMode::SwitchSides()
{
	for (int i = 0; i < TSGameState->TeamData.Num(); i++)
	{
		FTeamData& CurrentTeam = TSGameState->TeamData[i];
		int NewCourt = (static_cast<int>(CurrentTeam.AssignedCourt->GetCourtSide())) ? 0 : 1;
		CurrentTeam.AssignedCourt = TSGameState->GetCourt(static_cast<ECourtSide>(NewCourt));

		for (int j = 0; j < CurrentTeam.AssignedCharacters.Num(); j++)
		{
			ATennisStoryCharacter* Character = CurrentTeam.AssignedCharacters[j].Get();
			Character->CacheCourtAimVector(CurrentTeam.AssignedCourt->GetActorForwardVector());
		}
	}
}

TArray<FString> ATennisStoryGameMode::GenerateTeamNameArray()
{
	TArray<FString> TeamNameArray;

	for (int i = 0; i < TSGameState->TeamData.Num(); i++)
	{
		TeamNameArray.Add(TSGameState->TeamData[i].TeamName);
	}

	return TeamNameArray;
}
