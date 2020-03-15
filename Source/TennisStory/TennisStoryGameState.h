// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Gameplay/HalfCourt.h"
#include "Player/TennisStoryPlayerController.h"
#include "Player/TennisStoryCharacter.h"
#include "TennisStoryGameState.generated.h"

class ATennisStoryPlayerState;
class ATennisBall;
class UScoreboardWidget;
class UScoreCalloutWidget;
class UPlayerReadyStatusWidget;
class UReadyUpWidget;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStateAdded, ATennisStoryPlayerState*)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStateRemoved, ATennisStoryPlayerState*)

UENUM()
enum class EMatchState : uint8
{
	Uninitialized,
	WaitingForPlayers,
	MatchInProgress,
	WaitingForNextMatch
};

UENUM()
enum class EPlayState : uint8
{
	Service,
	PlayingPoint,
	Waiting
};

UENUM()
enum class EPointResolutionContext : uint8
{
	Point,
	Game,
	Set,
	Match
};

UENUM()
enum class EPointResolutionType : uint8
{
	Winner,
	Out,
	DoubleFault,
	IllegalHit
};

USTRUCT()
struct FTeamData
{
	GENERATED_BODY()

public:
	FTeamData()
	{
		TeamId = -1;
		TeamName = FString(TEXT("Default Team Name"));
	}

	FTeamData(int Id)
	{
		TeamId = Id;
		TeamName = FString(TEXT("Default Team Name"));
	}

	UPROPERTY()
	FString TeamName;

	UPROPERTY()
	int TeamId;
	
	UPROPERTY()
	TArray<TWeakObjectPtr<ATennisStoryPlayerController>> AssignedPlayers;
	
	UPROPERTY()
	TArray<TWeakObjectPtr<ATennisStoryCharacter>> AssignedCharacters;
	
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

		bHasBeenDeuce = false;
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

	bool IsCurrentlyDeuce() const
	{
		if (Scores[0] >= 3 && Scores[0] == Scores[1])
		{
			return true;
		}

		return false;
	}

	bool IsCurrentlyAd(int& OutAdTeamId) const
	{
		if (bHasBeenDeuce && FMath::Abs(Scores[0] - Scores[1]) == 1 && (Scores[0] >= 4 || Scores[1] >= 4))
		{
			OutAdTeamId = (Scores[0] > Scores[1]) ? 0 : 1;
			return true;
		}

		return false;
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
	FOnPlayerStateAdded& OnPlayerStateAdded(){ return PlayerStateAddedEvent; }
	FOnPlayerStateRemoved& OnPlayerStateRemoved(){ return PlayerStateRemovedEvent; }

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

	const FTeamData GetTeamForCharacter(ATennisStoryCharacter* Character);

	const int GetTeamIdForCharacter(ATennisStoryCharacter* Character);

	const TWeakObjectPtr<AHalfCourt> GetCourtToAimAtForPlayer(ATennisStoryPlayerController* Player);

	const TWeakObjectPtr<AHalfCourt> GetCourtForPlayer(ATennisStoryPlayerController* Player);

	const TWeakObjectPtr<AHalfCourt> GetCourtToAimAtForCharacter(ATennisStoryCharacter* Character);

	const TWeakObjectPtr<AHalfCourt> GetCourtForCharacter(ATennisStoryCharacter* Character);

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
	FString GetGameScoreDisplayStringForTeam(int TeamId) const
	{
		return GetDisplayStringForCurrentGameScoreByTeam(TeamId);
	}
	
	void AddScoreWidgetToViewport();
	
	void RemoveScoreWidgetFromViewport();

	UPROPERTY()
	UScoreboardWidget* ScoreboardWidgetObject;
	
	UFUNCTION(NetMulticast, Reliable)
	void AddCalloutWidgetToViewport(float ShowDuration, const FText& HeaderText, const FText& BodyText);

	UFUNCTION()
	void RemoveCalloutWidgetFromViewport();

	UPROPERTY()
	UScoreCalloutWidget* ScoreCalloutWidgetObject;

	EPlayState GetCurrentPlayState() const
	{
		return CurrentPlayState;
	}
	
	virtual void AddPlayerState(APlayerState* PlayerState) override;

	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	UPROPERTY()
	UPlayerReadyStatusWidget* PlayerReadyStateWidgetObject;

	void AddReadyStateWidgetToViewport();
	
	void RemoveReadyStateWidgetFromViewport();

	UPROPERTY()
	UReadyUpWidget* ReadyUpWidgetObject;

	UPROPERTY()
	UReadyUpWidget* PlayAgainWidgetObject;

	void AddReadyUpWidgetToViewport();

	void RemoveReadyUpWidgetFromViewport();

	void AddPlayAgainWidgetToViewport();

	void RemovePlayAgainWidgetFromViewport();

	void SetLocalPlayerToUIInputMode();

	void SetLocalPlayerToGameInputMode();

protected:

	UPROPERTY(Transient, ReplicatedUsing = OnRep_MatchState)
	EMatchState CurrentMatchState;

	UFUNCTION()
	void OnRep_MatchState();

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

	UPROPERTY(Transient, Replicated)
	int NumSets;

	UPROPERTY(EditDefaultsOnly, Category = "Score UI")
	TSubclassOf<UScoreboardWidget> ScoreboardWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Score UI")
	TSubclassOf<UScoreCalloutWidget> ScoreCalloutWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Match State")
	TSubclassOf<UPlayerReadyStatusWidget> PlayerReadyStateWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Match State")
	TSubclassOf<UReadyUpWidget> ReadyUpWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Match State")
	TSubclassOf<UReadyUpWidget> PlayAgainWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Teams")
	TArray<FString> TeamNames;

	//Score Display String Functions

	//Returns a display string for the current game score for the provided team
	FString GetDisplayStringForCurrentGameScoreByTeam(int TeamId) const;

	//Returns a display string to describe the current game score for both teams (EX: 30 - 15, DEUCE, etc)
	FString GetDisplayStringForCurrentGameScoreFull() const;

	//Returns a display string to describe the current set score (game count) for both teams
	FString GetDisplayStringForSetScore(int SetNum) const;

	//Returns a display string to describe the match score (set count) for both teams
	FString GetDisplayStringForMatchScoreShort() const;

	int GetNumCompletedSets() const;

	//Returns a display string to describe the match score (full game counts for each set) for both teams
	FString GetDisplayStringForMatchScoreLong() const;

	//End Score Display String Functions

	FOnPlayerStateAdded PlayerStateAddedEvent;
	FOnPlayerStateRemoved PlayerStateRemovedEvent;

	friend class ATennisStoryGameMode;
};
