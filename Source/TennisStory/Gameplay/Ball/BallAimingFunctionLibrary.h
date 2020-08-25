// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include <Engine/DataTable.h>
#include "BallAimingFunctionLibrary.generated.h"

class USplineComponent;

UENUM()
enum class ETrajectoryAlgorithm : uint8
{
	Old,
	New
};

USTRUCT(BlueprintType)
struct FTrajectoryParams_New : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	UCurveFloat* TrajectoryCurve; 
};

USTRUCT(BlueprintType)
struct FTrajectoryParams_Old : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	UCurveFloat* TrajectoryCurve; 
	
	UPROPERTY(EditAnywhere)
	float ApexHeight = 200.f; 
	
	UPROPERTY(EditAnywhere)
	float TangentLength = 500.f;
};

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

	UPROPERTY()
	FVector TrajectoryEndLocation;

	void AddTrajectoryPoint(FVector PointLocation, FVector PointTangent);
};

UCLASS()
class TENNISSTORY_API UBallAimingFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static FBallTrajectoryData GenerateTrajectoryData(FTrajectoryParams_New TrajParams_New, FVector StartLocation, FVector EndLocation);

	static FBallTrajectoryData GenerateTrajectoryData(FTrajectoryParams_Old TrajParams_Old, FVector StartLocation, FVector EndLocation);

	static FBallTrajectoryData GenerateTrajectoryData(UCurveFloat* TrajectoryCurve, FVector StartLocation, FVector EndLocation, float ApexHeight = 200.f, float TangentLength = 500.f);
	
	static void ApplyTrajectoryDataToSplineComp(FBallTrajectoryData& TrajectoryData, USplineComponent* SplineComp);

	static void DebugVisualizeSplineComp(USplineComponent* SplineComp);
};
