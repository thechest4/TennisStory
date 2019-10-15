#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Gameplay/HalfCourt.h"
#include "TennisStoryGameMode.generated.h"

class ATennisBall;

UCLASS(minimalapi)
class ATennisStoryGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATennisStoryGameMode();

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

	UFUNCTION(BlueprintCallable, Category = "Tennis Court")
	AHalfCourt* GetCourtForPlayer(APlayerController* PlayerController) const
	{
		return GetCourt(static_cast<ECourtSide>(PlayerController->NetPlayerIndex));
	}

	virtual void StartPlay() override;

	virtual void RestartPlayer(AController* NewPlayer) override;

	virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;

	TWeakObjectPtr<AHalfCourt> FindPlayerCourt(AController* NewPlayer);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Classes)
	TSubclassOf<ATennisBall> DefaultBallClass;

	UPROPERTY()
	TWeakObjectPtr<ATennisBall> CurrentBallActor;

	UPROPERTY()
	TArray<TWeakObjectPtr<AHalfCourt>> Courts;

	void GetCourtsFromWorld();
};



