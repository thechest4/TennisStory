// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Gameplay/HalfCourt.h"
#include "Player/TennisStoryPlayerController.h"
#include "TennisStoryGameState.generated.h"

class ATennisBall;
class ATennisStoryCharacter;
class UScoreboardWidget;

UENUM()
enum class EPlayState : uint8
{
	Service,
	PlayingPoint,
	Waiting
};

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
		bHasBeenDeuce = false;
	}

	UPROPERTY()
	TArray<int> Scores;

	UPROPERTY()
	bool bHasBeenDeuce;

	void AddPoint(int TeamId);

	void ResetScore()
	{
		for (int i = 0; i < Scores.Num(); i++)
		{
			Scores[i] = 0;
		}

		bHasBeenDeuce = true;
	}

	EServiceSide GetServiceSide()
	{
		int TotalNumPoints = 0;

		for (int i = 0; i < Scores.Num(); i++)
		{
			TotalNumPoints += Scores[i];
		}

		int Side = TotalNumPoints % 2;

		return static_cast<EServiceSide>(Side);
	}

	FString GetDisplayStringForScore(int TeamId) const;
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
	
	TWeakObjectPtr<ATennisStoryCharacter> GetServingCharacter() const { return CurrentServingCharacter; }

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

	void InitScores(int NumTeams, int NumSets);

	void GetSetScores(int SetNum, TArray<int>& OutScores);

	const FTeamData GetTeamForPlayer(ATennisStoryPlayerController* Player);

	const int GetTeamIdForPlayer(ATennisStoryPlayerController* Player);

	const TWeakObjectPtr<AHalfCourt> GetCourtToAimAtForPlayer(ATennisStoryPlayerController* Player);

	const TWeakObjectPtr<AHalfCourt> GetCourtForPlayer(ATennisStoryPlayerController* Player);

	void AwardPoint(int TeamId);

	void AwardGame(int TeamId);

	int GetTotalGameCountForCurrentSet();

	EServiceSide GetServiceSide()
	{
		return CurrentGameScore.GetServiceSide();
	}

	UFUNCTION(BlueprintCallable, Category = "Score")
	int GetGameScore(int TeamId) const
	{
		return CurrentGameScore.Scores[TeamId];
	}

	UFUNCTION(BlueprintCallable, Category = "Score")
	int GetSetScore(int TeamId, int SetNum) const
	{
		return CurrentMatchScores[TeamId].SetScores[SetNum];
	}
	
	UFUNCTION(BlueprintCallable, Category = "Score")
	int GetNumSets() const
	{
		return NumSets;
	}

	UFUNCTION(BlueprintCallable, Category = "Score")
	FString GetScoreDisplayString(int TeamId) const
	{
		return CurrentGameScore.GetDisplayStringForScore(TeamId);
	}
	
	void AddScoreWidgetToViewport();

	UPROPERTY()
	UScoreboardWidget* ScoreboardWidgetObject;

	EPlayState GetCurrentPlayState() const
	{
		return CurrentPlayState;
	}

protected:

	UPROPERTY(Transient, Replicated)
	EPlayState CurrentPlayState;

	UPROPERTY(Transient, Replicated)
	int CurrentServiceTeam;
	
	UPROPERTY(Transient, Replicated)
	int CurrentFaultCount;

	UPROPERTY(Transient, Replicated)
	TWeakObjectPtr<ATennisStoryCharacter> CurrentServingCharacter;

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

	UPROPERTY(Transient, ReplicatedUsing = OnRep_NumSets)
	int NumSets;

	UPROPERTY(EditDefaultsOnly, Category = "Score UI")
	TSubclassOf<UScoreboardWidget> ScoreboardWidgetClass;

	UFUNCTION()
	void OnRep_NumSets();

	friend class ATennisStoryGameMode;
};
