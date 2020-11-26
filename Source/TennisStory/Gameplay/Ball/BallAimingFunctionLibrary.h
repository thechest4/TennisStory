// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include <Engine/DataTable.h>
#include <GameplayTagContainer.h>
#include "BallAimingFunctionLibrary.generated.h"

class USplineComponent;

USTRUCT(BlueprintType)
struct FTrajectoryParams : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "HitTrajectory")
	UCurveFloat* TrajectoryCurve;

	UPROPERTY(EditAnywhere, Category = "GameplayTags", BlueprintReadOnly)
	FGameplayTagContainer ContextTags;

	UPROPERTY(EditAnywhere, Category = "GameplayTags", BlueprintReadOnly)
	FGameplayTag ShotTypeTag;

	UPROPERTY(EditAnywhere, Category = "AdjustUp")
	bool bCanBeAdjustedUpwards = true;

	UPROPERTY(EditAnywhere, Category = "AdjustUp", meta = (EditCondition = bCanBeAdjustedUpwards))
	float MinNetClearance = 10.f;

	UPROPERTY(EditAnywhere, Category = "AdjustUp", meta = (EditCondition = bCanBeAdjustedUpwards))
	int MinAdjustmentIndex = 5;

	UPROPERTY(EditAnywhere, Category = "AdjustUp", meta = (EditCondition = bCanBeAdjustedUpwards))
	int MaxAdjustmentIndex = 15;

	UPROPERTY(EditAnywhere, Category = "AdjustDown")
	bool bCanBeAdjustedDownwards = true;

	UPROPERTY(EditAnywhere, Category = "AdjustDown", meta = (EditCondition = bCanBeAdjustedDownwards))
	int MaxHeightConformingIndex = 5;

	UPROPERTY(EditAnywhere, Category = "BounceTrajectory")
	UCurveFloat* BounceTrajectoryCurve;

	UPROPERTY(EditAnywhere, Category = "BounceTrajectory")
	float BaseBounceHeight = 65.f;

	UPROPERTY(EditAnywhere, Category = "BounceTrajectory")
	float BounceLengthProportion = 1.f;
};

USTRUCT()
struct FBallTrajectoryPoint
{
	GENERATED_BODY()

public:
	FBallTrajectoryPoint()
	{
	}

	FBallTrajectoryPoint(FVector PointLocation)
	{
		Location = PointLocation;
	}

	FBallTrajectoryPoint(FVector PointLocation, FVector PointArriveTangent, FVector PointLeaveTangent)
	{
		Location = PointLocation;
		ArriveTangent = PointArriveTangent;
		LeaveTangent = PointLeaveTangent;
	}

	UPROPERTY()
	FVector Location = FVector::ZeroVector;

	UPROPERTY()
	FVector ArriveTangent = FVector::ZeroVector;

	UPROPERTY()
	FVector LeaveTangent = FVector::ZeroVector;

	UPROPERTY()
	bool bSetTangent = false;
};

USTRUCT()
struct FBallTrajectoryData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FGameplayTag ShotTypeTag;

	UPROPERTY()
	TArray<FBallTrajectoryPoint> TrajectoryPoints;

	UPROPERTY()
	int BounceLocationIndex;

	UPROPERTY()
	FVector TrajectoryEndLocation;

	UPROPERTY()
	//true when the trajectory needed adjustment to clear the net (or would have, in the case of trajectories that don't get adjusted)
	bool bShouldBeValidated = false;

	UPROPERTY()
	//Index to start validation from.  Equal to adjustment index when adjustment takes place
	int ValidateFromIndex = 0;

	void AddTrajectoryPoint(FVector PointLocation);

	void AddTrajectoryPoint(FVector PointLocation, FVector PointArriveTangent, FVector PointLeaveTangent);
};

UCLASS()
class TENNISSTORY_API UBallAimingFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static FBallTrajectoryData GenerateTrajectoryData(FTrajectoryParams TrajParams, FVector StartLocation, FVector EndLocation, AActor* WorldContextActor = nullptr);

	static void ApplyTrajectoryDataToSplineComp(FBallTrajectoryData& TrajectoryData, USplineComponent* SplineComp);

	static bool ValidateTrajectorySplineComp(FBallTrajectoryData& TrajectoryData, USplineComponent* SplineComp);

	static FTrajectoryParams RetrieveTrajectoryParamsFromDataProvider(FGameplayTag SourceTag, FGameplayTagContainer ContextTags, FGameplayTag ShotTypeTag, FGameplayTag FallbackTypeTag);

	static void DebugVisualizeSplineComp(USplineComponent* SplineComp);
};
