// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BallAimingFunctionLibrary.generated.h"

class USplineComponent;

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

	UPROPERTY()
	float ApexHeight;

	UPROPERTY()
	float TrajectoryDistance;

	void AddTrajectoryPoint(FVector PointLocation, FVector PointTangent);
};

UCLASS()
class TENNISSTORY_API UBallAimingFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static FBallTrajectoryData GenerateTrajectoryData(UCurveFloat* TrajectoryCurve, FVector StartLocation, FVector EndLocation, float ApexHeight = 200.f, float TangentLength = 500.f);
	
	static void ApplyTrajectoryDataToSplineComp(FBallTrajectoryData& TrajectoryData, USplineComponent* SplineComp);

	static void DebugVisualizeSplineComp(USplineComponent* SplineComp);
};
