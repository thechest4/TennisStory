// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Gameplay/HalfCourt.h"
#include "Player/TennisStoryPlayerController.h"
#include "TennisStoryGameState.generated.h"

class ATennisBall;

USTRUCT()
struct FTeamData
{
	GENERATED_BODY()

public:
	FTeamData()
	{
		TeamId = -1;
	}

	FTeamData(int Id)
	{
		TeamId = Id;
	}

	UPROPERTY()
	int TeamId;
	
	UPROPERTY()
	TArray<TWeakObjectPtr<ATennisStoryPlayerController>> AssignedPlayers;
	
	UPROPERTY()
	TWeakObjectPtr<AHalfCourt> AssignedCourt;
};

USTRUCT()
struct FGameScore
{
	GENERATED_BODY()

public:
	FGameScore(){}

	FGameScore(int NumTeams)
	{
		Scores.Init(0, NumTeams);
	}

	UPROPERTY()
	TArray<int> Scores;

	void AddPoint(int TeamId);

	void ResetScore()
	{
		for (int i = 0; i < Scores.Num(); i++)
		{
			Scores[i] = 0;
		}
	}

	EServiceSide GetNextServiceSide()
	{
		int TotalNumPoints = 0;

		for (int i = 0; i < Scores.Num(); i++)
		{
			TotalNumPoints += Scores[i];
		}

		int Side = TotalNumPoints % 2;

		return static_cast<EServiceSide>(Side);
	}
};

USTRUCT()
struct FMatchScore
{
	GENERATED_BODY()

public:
	FMatchScore()
	{
		SetScores.Add(0);
		SetsWon = 0;
	}

	UPROPERTY()
	TArray<int> SetScores;

	UPROPERTY()
	int SetsWon;
};

UCLASS()
class TENNISSTORY_API ATennisStoryGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	TWeakObjectPtr<ATennisBall> GetTennisBall() const { return CurrentBallActor; }

	TArray<TWeakObjectPtr<AHalfCourt>> GetAllCourts() const
	{
		return Courts;
	}

	UFUNCTION(BlueprintCallable, Category = "Tennis Court")
	AHalfCourt* GetCourt(ECourtSide Side) const
	{
		for (TWeakObjectPtr<AHalfCourt> Court : Courts)
		{
			if (Court.IsValid() && Court->GetCourtSide() == Side)
			{
				return Court.Get();
			}
		}

		return nullptr;
	}

	void InitScores(int NumTeams);

	void StartNewSet();

	void GetSetScores(int SetNum, TArray<int>& OutScores);

	const FTeamData GetTeamForPlayer(ATennisStoryPlayerController* Player);

	const int GetTeamIdForPlayer(ATennisStoryPlayerController* Player);

	const TWeakObjectPtr<AHalfCourt> GetCourtToAimAtForPlayer(ATennisStoryPlayerController* Player);

	void AwardPoint(int TeamId);

	void AwardGame(int TeamId);

	int GetTotalGameCountForCurrentSet();

	EServiceSide GetServiceSideForNextPoint()
	{
		return CurrentGameScore.GetNextServiceSide();
	}

protected:
	UPROPERTY(Transient, Replicated)
	int CurrentServiceTeam;

	UPROPERTY(Transient, Replicated)
	TArray<FTeamData> TeamData;

	UPROPERTY(Transient, Replicated)
	FGameScore CurrentGameScore;
	
	UPROPERTY(Transient, Replicated)
	TArray<FMatchScore> CurrentMatchScores;

	UPROPERTY(Transient, Replicated)
	int CurrentSet;

	UPROPERTY(Transient, Replicated)
	TWeakObjectPtr<ATennisBall> CurrentBallActor;

	UPROPERTY(Transient, Replicated)
	TArray<TWeakObjectPtr<AHalfCourt>> Courts;

	friend class ATennisStoryGameMode;
};
