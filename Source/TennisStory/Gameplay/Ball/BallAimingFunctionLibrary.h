// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BallTrajectoryTypes.h"
#include "BallAimingFunctionLibrary.generated.h"

class USplineComponent;
class ATennisBall;

UCLASS()
class TENNISSTORY_API UBallAimingFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static FBallTrajectoryData GenerateTrajectoryData(FTrajectoryParams TrajParams, FVector StartLocation, FVector EndLocation, ATennisBall* TennisBallActor = nullptr);

	static void ApplyTrajectoryDataToSplineComp(FBallTrajectoryData& TrajectoryData, USplineComponent* SplineComp);

	static bool ValidateTrajectorySplineComp(FBallTrajectoryData& TrajectoryData, USplineComponent* SplineComp);

	static FTrajectoryParams RetrieveTrajectoryParamsFromDataProvider(FGameplayTag SourceTag, FGameplayTagContainer ContextTags, FGameplayTag ShotTypeTag, FGameplayTag FallbackTypeTag);

	static void DebugVisualizeSplineComp(USplineComponent* SplineComp);
};
