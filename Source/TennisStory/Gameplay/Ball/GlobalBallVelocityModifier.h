// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BallTrajectoryTypes.h"
#include "GlobalBallVelocityModifier.generated.h"

UCLASS()
class TENNISSTORY_API UGlobalBallVelocityModifier : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static void CalculateGlobalVelocityModifiers(FBallTrajectoryData TrajectoryData, TArray<float>& MultiplicativeModifiers);

	//Modifies velocity based on the distance of a shot
	static float CalculateDistanceModifier(FBallTrajectoryData TrajectoryData);

	//Modifies velocity based on whether the shot was adjusted upwards
	static float CalculateAdjustmentModifier(FBallTrajectoryData TrajectoryData);
};
