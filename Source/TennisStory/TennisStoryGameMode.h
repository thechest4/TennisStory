#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Gameplay/HalfCourt.h"
#include "TennisStoryGameMode.generated.h"

class ATennisStoryGameState;
class ATennisBall;

UCLASS(minimalapi)
class ATennisStoryGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATennisStoryGameMode();
	
	virtual void InitGameState() override;

	virtual void StartPlay() override;

	virtual void RestartPlayer(AController* NewPlayer) override;

	virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;

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

	UPROPERTY(Transient)
	ATennisStoryGameState* TSGameState;

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<ATennisStoryCharacter>> AllCharacters;

	void GetCourtsFromWorld();

	void GetCamPositioningCompFromWorld();

	TWeakObjectPtr<class UCamPositioningComponent> CameraPositioningComp;

	int AllowedBounces;

	int AllowedFaults;

	void HandleBallOutOfBounds(EBoundsContext BoundsContext);

	void HandleBallHitBounceLimit();

	void ResolvePoint(bool bLastPlayerWon);

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

private:
	const int MaxTeamNumber = 2;
};



