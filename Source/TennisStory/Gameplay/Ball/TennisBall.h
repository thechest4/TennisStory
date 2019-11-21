// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player/Components/BallStrikingComponent.h"
#include "Gameplay/Ball/BallMovementComponent.h"
#include "TennisBall.generated.h"

class ATennisStoryCharacter;

UENUM(BlueprintType)
enum class ETennisBallState : uint8
{
	ServiceState,
	PlayState
};

UCLASS()
class TENNISSTORY_API ATennisBall : public AActor
{
	GENERATED_BODY()
	
public:	
	DECLARE_EVENT_OneParam(ATennisBall, FOnBallSpawnedEvent, ATennisBall*);
	static FOnBallSpawnedEvent& OnBallSpawned(){ return BallSpawnedEvent; }

	ATennisBall();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UBallMovementComponent* BallMovementComp;

	UFUNCTION(BlueprintCallable, Category = "Tennis Ball")
	bool IsInServiceState();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Tennis Ball")
	void SetBallState(ETennisBallState NewState);

	TWeakObjectPtr<ATennisStoryCharacter> LastPlayerToHit;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_FollowPath(FBallTrajectoryData TrajectoryData, float Velocity, bool bFromHit);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnBounceParticleEffect(FVector Location);

	FVector GetCurrentDirection() const
	{
		if (!BallMovementComp)
		{
			return FVector::ZeroVector;
		}

		return BallMovementComp->GetCurrentDirection();
	}

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BallMesh;

	UPROPERTY()
	USplineComponent* BallTrajectorySplineComp;

	ETennisBallState CurrentBallState;

	void ApplyBallState();
	
	UPROPERTY(EditDefaultsOnly, Category = "Bounce")
	UParticleSystem* BounceParticleEffect;

private:
	static FOnBallSpawnedEvent BallSpawnedEvent;
};
