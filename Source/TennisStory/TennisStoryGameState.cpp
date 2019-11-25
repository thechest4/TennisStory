// Fill out your copyright notice in the Description page of Project Settings.

#include "TennisStoryGameState.h"
#include "Net/UnrealNetwork.h"

void ATennisStoryGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATennisStoryGameState, CurrentBallActor);
	DOREPLIFETIME(ATennisStoryGameState, Courts);
	DOREPLIFETIME(ATennisStoryGameState, TeamData);
	DOREPLIFETIME(ATennisStoryGameState, CurrentGameScore);
	DOREPLIFETIME(ATennisStoryGameState, CurrentMatchScores);
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

void ATennisStoryGameState::AwardPoint(int TeamId)
{
	CurrentGameScore.AddPoint(TeamId);
}

void ATennisStoryGameState::AwardGame(int TeamId)
{
	CurrentMatchScores[TeamId].SetScores[CurrentSet]++;

	CurrentGameScore.ResetScore();
	
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, FString::Printf(TEXT("ATennisStoryGameState::AwardGame - Awarded game to team %d, Set Score is now %d | %d"), TeamId, CurrentMatchScores[0].SetScores[CurrentSet], CurrentMatchScores[1].SetScores[CurrentSet]));
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
	
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, FString::Printf(TEXT("FGameScore::AddPoint - Awarded point to team %d, Score is now %d | %d"), TeamId, Scores[0], Scores[1]));
}
