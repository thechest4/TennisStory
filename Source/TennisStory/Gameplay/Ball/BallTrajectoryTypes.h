#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include <Engine/DataTable.h>
#include <GameplayTagContainer.h>
#include "BallTrajectoryTypes.generated.h"

USTRUCT(BlueprintType)
//Contains the rules for calculating the global velocity modifier based on distance
struct FDistanceModifierRules
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Velocity Modifiers | Distance")
	bool bEnableDistanceModifier = true;

	UPROPERTY(EditAnywhere, Category = "Velocity Modifiers | Distance", meta = (EditCondition = bEnableDistanceModifier))
	float ReferenceDistance = 1000.f;

	UPROPERTY(EditAnywhere, Category = "Velocity Modifiers | Distance", meta = (EditCondition = bEnableDistanceModifier))
	UCurveFloat* ModifierCurve;
};

USTRUCT(BlueprintType)
struct FTrajectoryParams : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "HitTrajectory")
	UCurveFloat* TrajectoryCurve;

	UPROPERTY(EditAnywhere, Category = "HitTrajectory")
	float BaseVelocity = 1500.f;

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

	UPROPERTY(EditAnywhere, Category = "HeightOverride")
	bool bOverrideHeight = false;

	UPROPERTY(EditAnywhere, Category = "HeightOverride", meta = (EditCondition = bOverrideHeight))
	float HeightOverride = 500.f;

	UPROPERTY(EditAnywhere, Category = "BounceTrajectory")
	UCurveFloat* BounceTrajectoryCurve;

	UPROPERTY(EditAnywhere, Category = "BounceTrajectory")
	float BounceBaseVelocity = 1200.f;

	UPROPERTY(EditAnywhere, Category = "BounceTrajectory")
	float BaseBounceHeight = 65.f;

	UPROPERTY(EditAnywhere, Category = "BounceTrajectory")
	float BounceLengthProportion = 1.f;

	UPROPERTY(EditAnywhere, Category = "Velocity Modifier")
	FDistanceModifierRules DistanceModifierRules;
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
	float BaseVelocity;

	UPROPERTY()
	float BaseBounceVelocity;

	UPROPERTY()
	float ModifiedVelocity;

	UPROPERTY()
	float ModifiedBounceVelocity;

	UPROPERTY()
	int BounceLocationIndex;

	UPROPERTY()
	FVector TrajectoryEndLocation;

	UPROPERTY()
	//2D distance from startlocation to endlocation
	float ShotDistance;

	UPROPERTY()
	//true when the trajectory needed adjustment to clear the net (or would have, in the case of trajectories that don't get adjusted)
	bool bShouldBeValidated = false;

	UPROPERTY()
	//Index to start validation from.  Equal to adjustment index when adjustment takes place
	int ValidateFromIndex = 0;

	UPROPERTY()
	FDistanceModifierRules DistanceModifierRules;

	void AddTrajectoryPoint(FVector PointLocation);

	void AddTrajectoryPoint(FVector PointLocation, FVector PointArriveTangent, FVector PointLeaveTangent);

	void ApplyVelocityModifiers(TArray<float> MultiplicativeModifiers);
};
