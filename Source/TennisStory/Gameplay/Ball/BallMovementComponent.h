// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BallMovementComponent.generated.h"

class USplineComponent;
class USplineMeshComponent;
class UCurveFloat;

UENUM(BlueprintType)
enum class EBallMovementState : uint8
{
	FollowingPath,
	Physical,
	NotMoving
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TENNISSTORY_API UBallMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBallMovementComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void FollowPath(USplineComponent* PathProviderComp, float Velocity, UCurveFloat* TrajectoryCurve);

	UFUNCTION(BlueprintCallable, Category = "Tennis Ball")
	void StopMoving();

	UFUNCTION(BlueprintCallable, Category = "Tennis Ball")
	EBallMovementState GetBallMovementState() const
	{
		return CurrentMovementState;
	}

protected:
	virtual void BeginPlay() override;

	void EnterPhysicalMovementState();

	UFUNCTION()
	void HandleActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY()
	UPrimitiveComponent* BallCollisionComponent;
	
	// Movement State Properties
	UPROPERTY(ReplicatedUsing = OnRep_CurrentMovementState)
	EBallMovementState CurrentMovementState;

	UFUNCTION()
	virtual void OnRep_CurrentMovementState();

	float Velocity;

	UPROPERTY()
	USplineComponent* PathProviderComp;

	UPROPERTY()
	UCurveFloat* TrajectoryCurve;
};
