// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/Ball/BallAimingFunctionLibrary.h"
#include "Gameplay/HalfCourt.h"
#include "BallMovementComponent.generated.h"

class ATennisBall;
class USplineComponent;
class USplineMeshComponent;
class UCurveFloat;

UENUM(BlueprintType)
enum class EBallMovementState : uint8
{
	FollowingPath,
	ContinueUntilHit,
	Physical,
	NotMoving,
	BounceLag,
	ServiceToss
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TENNISSTORY_API UBallMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBallMovementComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void StartFollowingPath(FBallTrajectoryData TrajectoryData, float Velocity, bool bIsFromHit);

	UFUNCTION(BlueprintCallable, Category = "Tennis Ball")
	void StopMoving();

	UFUNCTION(BlueprintCallable, Category = "Tennis Ball")
	EBallMovementState GetBallMovementState() const
	{
		return CurrentMovementState;
	}

	FVector GetCurrentDirection() const
	{
		return CurrentDirection;
	}

	void StartServiceToss(float TossHeight, float TotalTossDuration);

	void FinishServiceToss(bool bWasInterrupted = false);

	float GetCurrentTossAlpha() const
	{
		return CurrentTossAlpha;
	}

	void ProvideBoundsContext(EBoundsContext BoundsContext)
	{
		BoundsContextForFirstBounce = BoundsContext;
	}

protected:
	virtual void BeginPlay() override;

	void EnterPhysicalMovementState();

	UFUNCTION()
	void HandleActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY()
	UPrimitiveComponent* BallCollisionComponent;

	UPROPERTY()
	USplineComponent* TrajectorySplineComp;
	
	// Movement State Properties
	UPROPERTY(ReplicatedUsing = OnRep_CurrentMovementState)
	EBallMovementState CurrentMovementState;

	UFUNCTION()
	virtual void OnRep_CurrentMovementState();

	UPROPERTY()
	ATennisBall* OwnerPtr;

	float Velocity;
	int NumBounces;
	FVector CurrentDirection;

	//Bounce Properties
	void GenerateAndFollowBouncePath(const FHitResult& HitResult);

	UPROPERTY(EditDefaultsOnly, Category = "Bounce")
	UCurveFloat* BounceTrajectoryCurve;
	
	UPROPERTY(EditDefaultsOnly, Category = "Bounce")
	int FramesOfBounceLag;

	UPROPERTY(VisibleDefaultsOnly, Category = "Bounce")
	float DurationOfBounceLag;
	float CurrentLagTime;

	void DoBounceLag();

	EBoundsContext BoundsContextForFirstBounce;

	//Cached last path data
	float LastPathDistance;
	float LastPathHeight;

	//Current ball toss state info
	FVector TossStartLocation;
	FVector TossEndLocation;
	float CurrentTossAlpha;
	float TotalTossDuration;

	UPROPERTY(EditDefaultsOnly, Category = "Bounce SFX")
	TArray<USoundBase*> OrderedBounceSFX;
	
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	friend class ATennisBall;
};
