// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player/Components/BallStrikingComponent.h"
#include "Gameplay/Ball/BallMovementComponent.h"
#include "TennisBall.generated.h"

class ATennisStoryCharacter;

DECLARE_MULTICAST_DELEGATE(FOnBallOutOfBoundsEvent)
DECLARE_MULTICAST_DELEGATE(FOnBallHitBounceLimitEvent)

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

	FOnBallOutOfBoundsEvent& OnBallOutOfBounds(){ return BallOutOfBoundsEvent; }
	FOnBallHitBounceLimitEvent& OnBallHitBounceLimit(){ return BallHitBounceLimitEvent; }

	ATennisBall();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UBallMovementComponent* BallMovementComp;

	ETennisBallState GetCurrentBallState(){ return CurrentBallState; }

	UFUNCTION(BlueprintCallable, Category = "Tennis Ball")
	bool IsInServiceState();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Tennis Ball")
	void SetBallState(ETennisBallState NewState);

	TWeakObjectPtr<ATennisStoryCharacter> LastPlayerToHit;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_FollowPath(FBallTrajectoryData TrajectoryData, float Velocity, bool bFromHit);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnBounceParticleEffect(FVector Location);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnBounceLocationParticleEffect(FVector Location);

	FVector GetCurrentDirection() const
	{
		if (!BallMovementComp)
		{
			return FVector::ZeroVector;
		}

		return BallMovementComp->GetCurrentDirection();
	}

	float GetBallRadius() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BallMesh;

	UPROPERTY(VisibleAnywhere)
	class UDecalComponent* DropShadowDecal;

	UPROPERTY()
	USplineComponent* BallTrajectorySplineComp;

	ETennisBallState CurrentBallState;

	void ApplyBallState();
	
	UPROPERTY(EditDefaultsOnly, Category = "Bounce")
	UParticleSystem* BounceParticleEffect;
	
	UPROPERTY(EditDefaultsOnly, Category = "Bounce")
	UParticleSystem* BounceLocationParticleEffect;

private:
	static FOnBallSpawnedEvent BallSpawnedEvent;
	FOnBallOutOfBoundsEvent BallOutOfBoundsEvent;
	FOnBallHitBounceLimitEvent BallHitBounceLimitEvent;
};
