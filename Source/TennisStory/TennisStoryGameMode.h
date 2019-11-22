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

	UFUNCTION(BlueprintCallable, Category = "Game Management")
	void TeleportBallToCourt();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Classes)
	TSubclassOf<ATennisBall> DefaultBallClass;

	UPROPERTY(Transient)
	ATennisStoryGameState* TSGameState;

	void GetCourtsFromWorld();

	void GetCamPositioningCompFromWorld();

	TWeakObjectPtr<class UCamPositioningComponent> CameraPositioningComp;

	int AllowedBounces;

	UFUNCTION()
	void HandleBallOutOfBounds();

	UFUNCTION()
	void HandleBallHitBounceLimit();

	void ResolvePoint(bool bLastPlayerWon);

private:
	const int MaxTeamNumber = 2;
};



