// Fill out your copyright notice in the Description page of Project Settings.

#include "TennisStoryGameState.h"
#include "UI/Score/ScoreboardWidget.h"
#include "UI/Score/ScoreCalloutWidget.h"
#include "Net/UnrealNetwork.h"

void ATennisStoryGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

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
}

void ATennisStoryGameState::AwardGame(int TeamId)
{
	CurrentMatchScores[TeamId].SetScores[CurrentSet]++;

	CurrentGameScore.ResetScore();
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
		ScoreboardWidgetObject->AddToViewport();
	}
}

void ATennisStoryGameState::AddCalloutWidgetToViewport_Implementation(float ShowDuration, const FText& HeaderText, const FText& BodyText)
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
	
	ScoreCalloutWidgetObject->ShowCalloutWidget(ShowDuration, HeaderText, BodyText);
	ScoreCalloutWidgetObject->AddToViewport();

	ScoreCalloutWidgetObject->OnCalloutWidgetFinished().AddDynamic(this, &ATennisStoryGameState::RemoveCalloutWidgetFromViewport);
}

void ATennisStoryGameState::RemoveCalloutWidgetFromViewport()
{
	if (ScoreCalloutWidgetObject)
	{
		ScoreCalloutWidgetObject->RemoveFromViewport();
		ScoreCalloutWidgetObject->OnCalloutWidgetFinished().RemoveDynamic(this, &ATennisStoryGameState::RemoveCalloutWidgetFromViewport);
	}
}

void ATennisStoryGameState::OnRep_NumSets()
{
	AddScoreWidgetToViewport();
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

FString FGameScore::GetDisplayStringForScore(int TeamId) const
{
	static const FString ScoreDisplayValues[] = { TEXT("0"), TEXT("15"), TEXT("30"), TEXT("40") };
	static const FString DeuceString = TEXT("40");
	static const FString AdString = TEXT("Ad");
	static const FString DisAdString = TEXT("-"); //This is what your score will say if your opponent has the ad

	int OtherTeamId = (TeamId) ? 0 : 1;

	int MyScore = Scores[TeamId];
	int OtherTeamScore = Scores[OtherTeamId];
	
	if (IsCurrentlyDeuce())
	{
		return DeuceString;
	}

	int OutAdTeamId = -1;
	if (IsCurrentlyAd(OutAdTeamId))
	{
		return (TeamId == OutAdTeamId) ? AdString : DisAdString;
	}

	return ScoreDisplayValues[MyScore];
}

FString FGameScore::GetGameScoreDisplayString(TArray<FString>& TeamNameArray) const
{
	if (IsCurrentlyDeuce())
	{
		return FString(TEXT("DEUCE"));
	}
	
	int OutAdTeamId = -1;
	if (IsCurrentlyAd(OutAdTeamId))
	{
		checkf(OutAdTeamId >= 0, TEXT("FGameScore::GetGameScoreDisplayString - IsCurrentlyAd() returned true but provided an invalid team id!"))

		FString TeamName = FString(TEXT("No Team Name Provided"));
		if (OutAdTeamId < TeamNameArray.Num())
		{
			TeamName = TeamNameArray[OutAdTeamId];
		}

		return FString(TEXT("AD ")) + TeamName.ToUpper();
	}

	return GetDisplayStringForScore(0) + FString(TEXT(" - ")) + GetDisplayStringForScore(1);
}

