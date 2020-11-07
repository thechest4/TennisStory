// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player/Components/BallStrikingComponent.h"
#include "Gameplay/Ball/BallMovementComponent.h"
#include "Gameplay/HalfCourt.h"
#include "TennisBall.generated.h"

class ATennisStoryCharacter;
class UDistanceIndicatorComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnBallOutOfBoundsEvent, EBoundsContext, FVector)
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

	UPROPERTY(Replicated)
	bool bWasLastHitAServe;
	
	UPROPERTY(Replicated)
	TWeakObjectPtr<ATennisStoryCharacter> LastPlayerToHit;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_FollowPath(FBallTrajectoryData TrajectoryData, float Velocity, EBoundsContext BoundsContext, ATennisStoryCharacter* PlayerWhoHitBall);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnBounceParticleEffect(FVector Location);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnBounceLocationParticleEffect(FVector Location);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnHitParticleEffect(UParticleSystem* HitFX, FVector Location);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySound(USoundBase* Sound, FVector Location);

	FVector GetCurrentDirection() const
	{
		if (!BallMovementComp)
		{
			return FVector::ZeroVector;
		}

		return BallMovementComp->GetCurrentDirection();
	}

	float GetBallRadius() const;

	void StartServiceToss(float TossHeight, float TossDuration);

	void InterruptServiceToss();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_InterruptServiceToss();

	FORCEINLINE_DEBUGGABLE int GetCurrentNumBounces() const
	{
		return BallMovementComp->NumBounces;
	}

	TWeakObjectPtr<const USplineComponent> GetSplineComponent() { return BallTrajectorySplineComp; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BallMesh;

	UPROPERTY(VisibleAnywhere)
	class UDecalComponent* DropShadowDecal;

	UPROPERTY()
	USplineComponent* BallTrajectorySplineComp;

	UPROPERTY(ReplicatedUsing = OnRep_BallState)
	ETennisBallState CurrentBallState;

	UFUNCTION()
	void OnRep_BallState();

	void ApplyBallState();
	
	UPROPERTY(EditDefaultsOnly, Category = "Ball FX")
	UParticleSystem* BounceParticleEffect;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ball FX")
	UParticleSystem* BounceLocationParticleEffect;

	UPROPERTY(EditAnywhere, Category = "Ball FX")
	bool bTrailAlwaysOn;

	UPROPERTY(EditDefaultsOnly, Category = "Ball FX")
	UParticleSystemComponent* BallTrailParticleEffect;
	
	UPROPERTY(VisibleDefaultsOnly)
	UStaticMeshComponent* DistanceIndicatorRing;

	UPROPERTY(VisibleAnywhere)
	UDistanceIndicatorComponent* DistanceIndicatorComp;

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* DynamicBallMat;

	UFUNCTION()
	void HandleDistanceIndicatorTargetReached();

private:
	static FOnBallSpawnedEvent BallSpawnedEvent;
	FOnBallOutOfBoundsEvent BallOutOfBoundsEvent;
	FOnBallHitBounceLimitEvent BallHitBounceLimitEvent;
};
