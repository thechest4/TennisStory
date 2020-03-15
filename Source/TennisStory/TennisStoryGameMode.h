#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Gameplay/HalfCourt.h"
#include "TennisStoryGameState.h"
#include "TennisStoryGameMode.generated.h"

class UTennisStoryGameInstance;
class ATennisStoryGameState;
class ATennisBall;
class ABounceLocationMarker;
class ATennisStoryPlayerState;

UCLASS(minimalapi)
class ATennisStoryGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATennisStoryGameMode();
	
	virtual void InitGameState() override;

	virtual void StartPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Match State")
	void StartMatch();

	virtual void RestartPlayer(AController* NewPlayer) override;

	virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	int GetAllowedBounces() const
	{
		return AllowedBounces;
	}
	
	UFUNCTION(BlueprintCallable, Category = "Game Management")
	void TeleportCharacterToCourt(ATennisStoryCharacter* Character);

	UFUNCTION()
	void SetUpNextPoint();

	void ReportServeHit();

	void DetermineHitLegality(ATennisStoryCharacter* Character);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Classes)
	TSubclassOf<ATennisBall> DefaultBallClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Classes)
	TSubclassOf<ABounceLocationMarker> DefaultBounceMarkerClass;

	TWeakObjectPtr<ABounceLocationMarker> BounceMarkerActor;

	UPROPERTY(Transient)
	ATennisStoryGameState* TSGameState;

	UPROPERTY(Transient)
	UTennisStoryGameInstance* TSGameInstance;

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<ATennisStoryCharacter>> AllCharacters;

	void GetCourtsFromWorld();

	void GetCamPositioningCompFromWorld();

	TWeakObjectPtr<class UCamPositioningComponent> CameraPositioningComp;

	int AllowedBounces;

	int AllowedFaults;

	void HandleBallOutOfBounds(EBoundsContext BoundsContext, FVector BounceLocation);

	void HandleBallHitBounceLimit();

	void ResolvePoint(bool bLastPlayerWon, bool bShowBounceLocation, FVector BounceLocation, EPointResolutionType PointType);

	void SwitchSides();

	//Match data params
	UPROPERTY(EditDefaultsOnly, Category = "Match Length")
	int NumSets = 3;
	
	UPROPERTY(EditDefaultsOnly, Category = "Match Length")
	int GamesToWinSet = 6;
	
	UPROPERTY(EditDefaultsOnly, Category = "Match Length")
	int MarginToWinSet = 2;
	
	UPROPERTY(EditDefaultsOnly, Category = "Match Length")
	int PointsToWinGame = 4;

	UPROPERTY(EditDefaultsOnly, Category = "Match Length")
	int MarginToWinGame = 2;

	//Handling PlayerState readiness
	UFUNCTION()
	void HandlePlayerReadyStateUpdated(ATennisStoryPlayerState* PlayerState);

	UFUNCTION()
	void HandlePlayerStateAdded(ATennisStoryPlayerState* PlayerState);
	
	UFUNCTION()
	void HandlePlayerStateRemoved(ATennisStoryPlayerState* PlayerState);

	void RegisterToPlayerReadyStateUpdates();

	//Match End handling
	UFUNCTION()
	void HandleMatchEnded();

	void EnterWaitingForNextMatchState();

private:
	const int MaxTeamNumber = 2;
};



