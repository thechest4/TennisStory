// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BallStrikingComponent.generated.h"

class UCurveFloat;
class USplineComponent;
class ATennisStoryCharacter;
class ATennisRacquet;
class APlayerTargetActor;

USTRUCT()
struct FBallTrajectoryPoint
{
	GENERATED_BODY()

public:
	FBallTrajectoryPoint()
	{
		Location = FVector::ZeroVector;
		Tangent = FVector::ZeroVector;
	}

	FBallTrajectoryPoint(FVector PointLocation, FVector PointTangent)
	{
		Location = PointLocation;
		Tangent = PointTangent;
	}

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FVector Tangent;
};

USTRUCT()
struct FBallTrajectoryData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FBallTrajectoryPoint> TrajectoryPoints;

	void AddTrajectoryPoint(FVector PointLocation, FVector PointTangent);
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TENNISSTORY_API UBallStrikingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBallStrikingComponent();

	void AllowBallStriking();

	void StopBallStriking();

	void SetChargeStartTime();

	void SetChargeEndTime();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleRacquetOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void GenerateTrajectorySpline();

	FBallTrajectoryData GetDataForCurrentSpline();

	void CopySplineFromData(FBallTrajectoryData TrajectoryData);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinBallSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxBallSpeed = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxChargeDuration = 3.0f;

	float CalculateChargedBallSpeed();

	//Charge State
	float LastChargeStartTime = 0.0f;
	float LastChargeEndTime = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = Aiming)
	UCurveFloat* TrajectoryCurve;

	//Cached Owner Pointers
	UPROPERTY()
	ATennisStoryCharacter* OwnerChar;
	
	UPROPERTY()
	ATennisRacquet* OwnerRacquet;

	UPROPERTY()
	APlayerTargetActor* OwnerTarget;

	UPROPERTY()
	USplineComponent* OwnerSplineComp;

	friend class ATennisStoryCharacter;
};
