// Fill out your copyright notice in the Description page of Project Settings.

#include "TennisStoryGameState.h"
#include "UI/PlayerWidgetManager.h"
#include "UI/Score/ScoreboardWidget.h"
#include "UI/Score/ScoreCalloutWidget.h"
#include "UI/Score/ServiceCalloutWidget.h"
#include "UI/MatchState/PlayerReadyStatusWidget.h"
#include "UI/MatchState/ReadyUpWidget.h"
#include "Player/TennisStoryPlayerState.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogTS_MatchState)
DEFINE_LOG_CATEGORY(LogTS_MatchUI)

void ATennisStoryGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ATennisStoryGameState, CurrentMatchLengthParams);
	DOREPLIFETIME(ATennisStoryGameState, CurrentMatchState);
	DOREPLIFETIME(ATennisStoryGameState, CurrentPlayState);
	DOREPLIFETIME(ATennisStoryGameState, CurrentServiceTeam);
	DOREPLIFETIME(ATennisStoryGameState, CurrentFaultCount);
	DOREPLIFETIME(ATennisStoryGameState, CurrentServingCharacter);
	DOREPLIFETIME(ATennisStoryGameState, TeamData);
	DOREPLIFETIME(ATennisStoryGameState, CurrentGameScore);
	DOREPLIFETIME(ATennisStoryGameState, CurrentMatchScores);
	DOREPLIFETIME(ATennisStoryGameState, CurrentSet);
	DOREPLIFETIME(ATennisStoryGameState, CurrentBallActor);
	DOREPLIFETIME(ATennisStoryGameState, Courts);
	DOREPLIFETIME(ATennisStoryGameState, NumSets);
}

void ATennisStoryGameState::InitScores(int NumTeams, int argNumSets)
{
	UE_LOG(LogTS_MatchState, Log, TEXT("ATennisStoryGameState::InitScores"))

	NumSets = argNumSets;
	CurrentSet = 0;
	CurrentGameScore = FGameScore(NumTeams);
	CurrentMatchScores.Init(FMatchScore(), NumTeams);
	
	for (int i = 0; i < NumTeams; i++)
	{
		CurrentMatchScores[i].SetScores.Init(0, NumSets);
	}
}

void ATennisStoryGameState::GetSetScores(int SetNum, TArray<int>& OutScores)
{
	OutScores.Init(0, CurrentMatchScores.Num());

	for (int i = 0; i < CurrentMatchScores.Num(); i++)
	{
		if (SetNum >= 0 && SetNum < CurrentMatchScores[i].SetScores.Num())
		{
			OutScores[i] = CurrentMatchScores[i].SetScores[SetNum];
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("ATennisStoryGameState::GetSetScores - Invalid Set Number"));
		}
	}
}

const FTeamData ATennisStoryGameState::GetTeamForPlayer(ATennisStoryPlayerController* Player)
{
	if (!TeamData.Num())
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("ATennisStoryGameState::GetTeamForPlayer - No TeamData found!"));
	}

	FTeamData PlayerTeam = TeamData[0];

	for (FTeamData& Team : TeamData)
	{
		if (Team.AssignedPlayers.Contains(Player))
		{
			PlayerTeam = Team;
			break;
		}
	}

	return PlayerTeam;
}

const int ATennisStoryGameState::GetTeamIdForPlayer(ATennisStoryPlayerController* Player)
{
	if (!TeamData.Num())
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("ATennisStoryGameState::GetTeamIdForPlayer - No TeamData found!"));
	}

	int IdToReturn = -1;

	for (FTeamData& Team : TeamData)
	{
		if (Team.AssignedPlayers.Contains(Player))
		{
			IdToReturn = Team.TeamId;
			break;
		}
	}

	return IdToReturn;
}

const FTeamData ATennisStoryGameState::GetTeamForCharacter(ATennisStoryCharacter* Character)
{
	if (!TeamData.Num())
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("ATennisStoryGameState::GetTeamForCharacter - No TeamData found!"));
	}

	FTeamData CharTeam = TeamData[0];

	for (FTeamData& Team : TeamData)
	{
		if (Team.AssignedCharacters.Contains(Character))
		{
			CharTeam = Team;
			break;
		}
	}

	return CharTeam;
}

const int ATennisStoryGameState::GetTeamIdForCharacter(ATennisStoryCharacter* Character)
{
	if (!TeamData.Num())
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("ATennisStoryGameState::GetTeamIdForCharacter - No TeamData found!"));
	}

	int IdToReturn = -1;

	for (FTeamData& Team : TeamData)
	{
		if (Team.AssignedCharacters.Contains(Character))
		{
			IdToReturn = Team.TeamId;
			break;
		}
	}

	return IdToReturn;
}

const TWeakObjectPtr<AHalfCourt> ATennisStoryGameState::GetCourtToAimAtForPlayer(ATennisStoryPlayerController* Player)
{
	TWeakObjectPtr<AHalfCourt> CourtToAimAt = nullptr;

	if (Player)
	{
		for (FTeamData& Team : TeamData)
		{
			if (!Team.AssignedPlayers.Contains(Player))
			{
				CourtToAimAt = Team.AssignedCourt;
				break;
			}
		}
	}

	return CourtToAimAt;
}

const TWeakObjectPtr<AHalfCourt> ATennisStoryGameState::GetCourtForPlayer(ATennisStoryPlayerController* Player)
{
	TWeakObjectPtr<AHalfCourt> Court = nullptr;

	if (Player)
	{
		for (FTeamData& Team : TeamData)
		{
			if (Team.AssignedPlayers.Contains(Player))
			{
				Court = Team.AssignedCourt;
				break;
			}
		}
	}

	return Court;
}

const TWeakObjectPtr<AHalfCourt> ATennisStoryGameState::GetCourtToAimAtForCharacter(ATennisStoryCharacter* Character)
{
	TWeakObjectPtr<AHalfCourt> CourtToAimAt = nullptr;

	if (Character)
	{
		for (FTeamData& Team : TeamData)
		{
			if (!Team.AssignedCharacters.Contains(Character))
			{
				CourtToAimAt = Team.AssignedCourt;
				break;
			}
		}
	}

	return CourtToAimAt;
}

const TWeakObjectPtr<AHalfCourt> ATennisStoryGameState::GetCourtForCharacter(ATennisStoryCharacter* Character)
{
	TWeakObjectPtr<AHalfCourt> Court = nullptr;

	if (Character)
	{
		for (FTeamData& Team : TeamData)
		{
			if (Team.AssignedCharacters.Contains(Character))
			{
				Court = Team.AssignedCourt;
				break;
			}
		}
	}

	return Court;
}

void ATennisStoryGameState::AwardPoint(int TeamId)
{
	CurrentGameScore.AddPoint(TeamId);

	UE_LOG(LogTS_MatchState, Log, TEXT("ATennisStoryGameState::AwardPoint - Point awarded to team %d, current game score: %s"), TeamId, *GetDisplayStringForCurrentGameScoreFull());
}

void ATennisStoryGameState::AwardGame(int TeamId)
{
	CurrentMatchScores[TeamId].SetScores[CurrentSet]++;
	
	CurrentGameScore.ResetScore();
	
	UE_LOG(LogTS_MatchState, Log, TEXT("ATennisStoryGameState::AwardGame - Game awarded to team %d, current set score: %s"), TeamId, *GetDisplayStringForSetScore(CurrentSet));
}

int ATennisStoryGameState::GetTotalGameCountForCurrentSet()
{
	int TotalGameCount = 0;

	for (int i = 0; i < CurrentMatchScores.Num(); i++)
	{
		TotalGameCount += CurrentMatchScores[i].SetScores[CurrentSet];
	}

	return TotalGameCount;
}

void ATennisStoryGameState::AddScoreWidgetToViewport()
{
	if (!ScoreboardWidgetClass)
	{
		return;
	}

	if (!ScoreboardWidgetObject)
	{
		ScoreboardWidgetObject = CreateWidget<UScoreboardWidget>(GetWorld(), ScoreboardWidgetClass);
		ScoreboardWidgetObject->AddSetScoreWidgets();
		ScoreboardWidgetObject->SetTeamNames(TeamData[0].TeamName, TeamData[1].TeamName);
	}
	
	if (!ScoreboardWidgetObject->IsInViewport())
	{
		ScoreboardWidgetObject->AddToViewport();
	}
}

void ATennisStoryGameState::RemoveScoreWidgetFromViewport()
{
	if (ScoreboardWidgetObject)
	{
		ScoreboardWidgetObject->RemoveFromViewport();
	}
}

void ATennisStoryGameState::AddCalloutWidgetToViewport_Implementation(float ShowDuration, const FText& HeaderText, const FText& BodyText, bool bShowSideSwitch)
{
	if (!ScoreCalloutWidgetClass)
	{
		return;
	}

	if (!ScoreCalloutWidgetObject)
	{
		ScoreCalloutWidgetObject = CreateWidget<UScoreCalloutWidget>(GetWorld(), ScoreCalloutWidgetClass);
	}

	checkf(ScoreCalloutWidgetObject, TEXT("ATennisStoryGameState::AddCalloutWidgetToViewport_Implementation - ScoreCalloutWidgetObject was null!"))
	
	ScoreCalloutWidgetObject->ShowCalloutWidget(ShowDuration, HeaderText, BodyText, bShowSideSwitch);

	if (!ScoreCalloutWidgetObject->IsInViewport())
	{
		ScoreCalloutWidgetObject->AddToViewport();
	}

	ScoreCalloutWidgetObject->OnCalloutWidgetFinished().AddDynamic(this, &ATennisStoryGameState::RemoveCalloutWidgetFromViewport);

	UE_LOG(LogTS_MatchUI, Log, TEXT("ATennisStoryGameState::AddCalloutWidgetToViewport_Implementation - %s | %s"), *HeaderText.ToString(), *BodyText.ToString())
}

void ATennisStoryGameState::RemoveCalloutWidgetFromViewport()
{
	if (ScoreCalloutWidgetObject)
	{
		ScoreCalloutWidgetObject->RemoveFromViewport();
		ScoreCalloutWidgetObject->OnCalloutWidgetFinished().RemoveDynamic(this, &ATennisStoryGameState::RemoveCalloutWidgetFromViewport);
	}
}

void ATennisStoryGameState::AddServiceWidgetToViewport_Implementation(float ShowDuration, const FText& HeaderText, const FText& BodyText)
{
	if (!ServiceWidgetClass)
	{
		return;
	}

	if (!ServiceWidgetObject)
	{
		ServiceWidgetObject = CreateWidget<UServiceCalloutWidget>(GetWorld(), ServiceWidgetClass);
	}

	checkf(ServiceWidgetObject, TEXT("ATennisStoryGameState::AddServiceWidgetToViewport_Implementation - ServiceWidgetObject was null!"))
	
	ServiceWidgetObject->ShowCalloutWidget(ShowDuration, HeaderText, BodyText);

	if (!ServiceWidgetObject->IsInViewport())
	{
		ServiceWidgetObject->AddToViewport();
	}

	ServiceWidgetObject->OnServiceCalloutWidgetFinished().AddDynamic(this, &ATennisStoryGameState::RemoveServiceWidgetFromViewport);

	UE_LOG(LogTS_MatchUI, Log, TEXT("ATennisStoryGameState::AddServiceWidgetToViewport_Implementation - %s | %s"), *HeaderText.ToString(), *BodyText.ToString())
}

void ATennisStoryGameState::RemoveServiceWidgetFromViewport()
{
	if (ServiceWidgetObject)
	{
		ServiceWidgetObject->RemoveFromViewport();
		ServiceWidgetObject->OnServiceCalloutWidgetFinished().RemoveDynamic(this, &ATennisStoryGameState::RemoveServiceWidgetFromViewport);
	}
}

void ATennisStoryGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	ATennisStoryPlayerState* TSPS = Cast<ATennisStoryPlayerState>(PlayerState);

	checkf(TSPS, TEXT("ATennisStoryGameState::AddPlayerState - PlayerState was not derived from ATennisStoryPlayerState"))

	OnPlayerStateAdded().Broadcast(TSPS);
}

void ATennisStoryGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
	
	ATennisStoryPlayerState* TSPS = Cast<ATennisStoryPlayerState>(PlayerState);

	checkf(TSPS, TEXT("ATennisStoryGameState::RemovePlayerState - PlayerState was not derived from ATennisStoryPlayerState"))

	OnPlayerStateRemoved().Broadcast(TSPS);
}

UPlayerWidgetManager* ATennisStoryGameState::GetLocalPlayerWidgetManager()
{
	for (FConstPlayerControllerIterator ControllerItr = GetWorld()->GetPlayerControllerIterator(); ControllerItr; ControllerItr++)
	{
		ATennisStoryPlayerController* TSPC = Cast<ATennisStoryPlayerController>(ControllerItr->Get());
		if (TSPC && TSPC->IsLocalPlayerController())
		{
			return TSPC->GetPlayerWidgetManager();
		}
	}

	checkNoEntry()

	return nullptr;
}

void ATennisStoryGameState::OnRep_MatchState()
{
	switch (CurrentMatchState)
	{
		default:
		case EMatchState::Uninitialized:
		{
			break;
		}
		case EMatchState::WaitingForPlayers:
		{
			GetLocalPlayerWidgetManager()->ShowReadyWidgets();

			RemoveScoreWidgetFromViewport();

			break;
		}
		case EMatchState::MatchInProgress:
		{
			GetLocalPlayerWidgetManager()->HideReadyWidgets();

			AddScoreWidgetToViewport();

			RemoveCalloutWidgetFromViewport();

			break;
		}
		case EMatchState::WaitingForNextMatch:
		{
			GetLocalPlayerWidgetManager()->ShowReadyWidgets();

			break;
		}
	}
}

void ATennisStoryGameState::OnRep_GameScore()
{
	UE_LOG(LogTS_MatchState, Log, TEXT("ATennisStoryGameState::OnRep_GameScore - Current Game Score: %s"), *GetRawDisplayStringForCurrentGameScore());
}

void ATennisStoryGameState::OnRep_MatchScore()
{
	UE_LOG(LogTS_MatchState, Log, TEXT("ATennisStoryGameState::OnRep_MatchScore - Current Match Score: %s"), *GetDisplayStringForMatchScoreLong(false));
}

void ATennisStoryGameState::OnRep_CurrentSet()
{
	UE_LOG(LogTS_MatchState, Log, TEXT("ATennisStoryGameState::OnRep_CurrentSet - Starting Set %d"), CurrentSet);
}

FString ATennisStoryGameState::GetDisplayStringForCurrentGameScoreByTeam(int TeamId) const
{
	static const FString ScoreDisplayValues[] = { TEXT("0"), TEXT("15"), TEXT("30"), TEXT("40") };
	static const FString DeuceString = TEXT("40");
	static const FString AdString = TEXT("Ad");
	static const FString DisAdString = TEXT("-"); //This is what your score will say if your opponent has the ad

	int OtherTeamId = (TeamId) ? 0 : 1;

	int MyScore = CurrentGameScore.Scores[TeamId];
	int OtherTeamScore = CurrentGameScore.Scores[OtherTeamId];
	
	if (CurrentGameScore.IsCurrentlyDeuce())
	{
		return DeuceString;
	}

	int OutAdTeamId = -1;
	if (CurrentGameScore.IsCurrentlyAd(OutAdTeamId))
	{
		return (TeamId == OutAdTeamId) ? AdString : DisAdString;
	}

	return ScoreDisplayValues[MyScore];
}

FString ATennisStoryGameState::GetDisplayStringForCurrentGameScoreFull() const
{
	if (CurrentGameScore.IsCurrentlyDeuce())
	{
		return FString(TEXT("DEUCE"));
	}

	int OutLeadingTeamIndex = -1;
	ECriticalPointType CriticalPointType = GetCriticalPointType(OutLeadingTeamIndex);
	switch (CriticalPointType)
	{
		default:
		case ECriticalPointType::None:
		{
			break;
		}
		case ECriticalPointType::GamePoint:
		{
			return FString(TEXT("GAME POINT"));
		}
		case ECriticalPointType::BreakPoint:
		{
			return FString(TEXT("BREAK POINT"));
		}
		case ECriticalPointType::SetPoint:
		{
			checkf(OutLeadingTeamIndex >= 0, TEXT("ATennisStoryGameState::GetDisplayStringForCurrentGameScoreFull - GetCriticalPointType() returned ECriticalPointType::SetPoint but provided an invalid team id!"))

			return FString(TEXT("SET POINT ")) + TeamData[OutLeadingTeamIndex].TeamName.ToUpper();
		}
		case ECriticalPointType::MatchPoint:
		{
			checkf(OutLeadingTeamIndex >= 0, TEXT("ATennisStoryGameState::GetDisplayStringForCurrentGameScoreFull - GetCriticalPointType() returned ECriticalPointType::MatchPoint but provided an invalid team id!"))

			return FString(TEXT("MATCH POINT ")) + TeamData[OutLeadingTeamIndex].TeamName.ToUpper();
		}
		case ECriticalPointType::DualGamePoint:
		{
			return FString(TEXT("DUAL GAME POINT"));
		}
		case ECriticalPointType::DualSetPoint:
		{
			return FString(TEXT("DUAL SET POINT"));
		}
		case ECriticalPointType::DualMatchPoint:
		{
			return FString(TEXT("DUAL MATCH POINT"));
		}
	}
	
	int OutAdTeamId = -1;
	if (CurrentGameScore.IsCurrentlyAd(OutAdTeamId))
	{
		checkf(OutAdTeamId >= 0, TEXT("ATennisStoryGameState::GetDisplayStringForCurrentGameScoreFull - IsCurrentlyAd() returned true but provided an invalid team id!"))

		return FString(TEXT("AD ")) + TeamData[OutAdTeamId].TeamName.ToUpper();
	}

	return GetDisplayStringForCurrentGameScoreByTeam(0) + FString(TEXT(" - ")) + GetDisplayStringForCurrentGameScoreByTeam(1);
}

FString ATennisStoryGameState::GetRawDisplayStringForCurrentGameScore() const
{
	return FString::FromInt(CurrentGameScore.Scores[0]) + FString(TEXT(" - ")) + FString::FromInt(CurrentGameScore.Scores[1]);
}

FString ATennisStoryGameState::GetDisplayStringForSetScore(int SetNum) const
{
	return FString::FromInt(CurrentMatchScores[0].SetScores[SetNum]) + FString(TEXT(" - ")) + FString::FromInt(CurrentMatchScores[1].SetScores[SetNum]);
}

FString ATennisStoryGameState::GetDisplayStringForMatchScoreShort() const
{
	return FString::FromInt(CurrentMatchScores[0].SetsWon) + FString(TEXT(" - ")) + FString::FromInt(CurrentMatchScores[1].SetsWon);
}

int ATennisStoryGameState::GetNumCompletedSets() const
{
	int NumCompletedSets = 0;

	for (int i = 0; i < CurrentMatchScores.Num(); i++)
	{
		NumCompletedSets += CurrentMatchScores[i].SetsWon;
	}

	return NumCompletedSets;
}

FString ATennisStoryGameState::GetDisplayStringForMatchScoreLong(bool bOnlyCompletedSets /*= true*/) const
{
	FString MatchScoreString = FString();

	const int SetsToDisplay = (bOnlyCompletedSets) ? GetNumCompletedSets() : NumSets;

	for (int i = 0; i < SetsToDisplay; i++)
	{
		FString SetScore = GetDisplayStringForSetScore(i);
		SetScore.RemoveSpacesInline();
		MatchScoreString += SetScore;

		if (i != SetsToDisplay - 1)
		{
			MatchScoreString += FString(TEXT(" | "));
		}
	}

	return MatchScoreString;
}

//NOTE(achester): The notion of a Dual CriticalPointType is an assumption of only 2 teams
ECriticalPointType ATennisStoryGameState::GetCriticalPointType(int& OutLeadingTeam) const
{
	bool bIsGamePoint = false;
	int LeadingTeamIndex = -1;
	int OtherTeamIndex = -1;
	bool bIsDualGamePoint = false;

	for (int i = 0; i < CurrentGameScore.Scores.Num(); i++)
	{
		if (CurrentGameScore.Scores[i] >= CurrentMatchLengthParams.PointsToWinGame - 1)
		{
			//NOTE(achester): Assumption of only 2 teams
			OtherTeamIndex = (i) ? 0 : 1;

			if (CurrentGameScore.Scores[i] - CurrentGameScore.Scores[OtherTeamIndex] >= CurrentMatchLengthParams.MarginToWinGame - 1)
			{
				if (bIsGamePoint)
				{
					//if we get to here but it was already a game point, that means both teams have game scoring opportunity
					bIsDualGamePoint = true;
				}
				else
				{
					bIsGamePoint = true;
					LeadingTeamIndex = i;
				}
			}
		}
	}

	if (!bIsGamePoint)
	{
		return ECriticalPointType::None;
	}

	bool bIsSetPoint = false;
	bool bIsDualSetPoint = false;

	if (bIsGamePoint)
	{
		if (!bIsDualGamePoint)
		{
			if (CurrentMatchScores[LeadingTeamIndex].SetScores[CurrentSet] >= CurrentMatchLengthParams.GamesToWinSet - 1 &&
				CurrentMatchScores[LeadingTeamIndex].SetScores[CurrentSet] - CurrentMatchScores[OtherTeamIndex].SetScores[CurrentSet] >= CurrentMatchLengthParams.MarginToWinSet - 1)
			{
				bIsSetPoint = true;
			}
		}
		else if (CurrentMatchLengthParams.MarginToWinSet <= 1)
		{
			for (int i = 0; i < CurrentMatchScores.Num(); i++)
			{
				if (CurrentMatchScores[i].SetScores[CurrentSet] >= CurrentMatchLengthParams.GamesToWinSet - 1)
				{
					if (bIsSetPoint)
					{
						bIsDualSetPoint = true;
					}
					else
					{
						bIsSetPoint = true;
						LeadingTeamIndex = i;
					}
				}
			}
		}
	}

	bool bIsMatchPoint = false;
	bool bIsDualMatchPoint = false;

	if (bIsSetPoint)
	{
		if (!bIsDualSetPoint)
		{
			if (CurrentMatchScores[LeadingTeamIndex].SetsWon == CurrentMatchLengthParams.SetsToWinMatch - 1)
			{
				bIsMatchPoint = true;
			}
		}
		else
		{
			for (int i = 0; i < CurrentMatchScores.Num(); i++)
			{
				if (CurrentMatchScores[i].SetsWon == CurrentMatchLengthParams.SetsToWinMatch - 1)
				{
					if (bIsMatchPoint)
					{
						bIsDualMatchPoint = true;
					}
					else
					{
						bIsMatchPoint = true;
						LeadingTeamIndex = i;
					}
				}
			}
		}
	}

	OutLeadingTeam = LeadingTeamIndex;

	if (bIsMatchPoint)
	{
		return (bIsDualMatchPoint) ? ECriticalPointType::DualMatchPoint : ECriticalPointType::MatchPoint;
	}
	else if (bIsSetPoint)
	{
		return (bIsDualSetPoint) ? ECriticalPointType::DualSetPoint : ECriticalPointType::SetPoint;
	}
	else
	{
		if (bIsDualGamePoint)
		{
			return ECriticalPointType::DualGamePoint;
		}

		return (LeadingTeamIndex == CurrentServiceTeam) ? ECriticalPointType::GamePoint : ECriticalPointType::BreakPoint;
	}
}

void FGameScore::AddPoint(int TeamId)
{
	if (!Scores.Num())
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("FGameScore::AddPoint - Scores array was empty!"));
		return;
	}

	if (TeamId < 0 || TeamId >= Scores.Num())
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("FGameScore::AddPoint - Invalid TeamId"));
		return;
	}

	Scores[TeamId]++;

	if (!bHasBeenDeuce)
	{
		if (IsCurrentlyDeuce())
		{
			bHasBeenDeuce = true;
		}
	}
}
