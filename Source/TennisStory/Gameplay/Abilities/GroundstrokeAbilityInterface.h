// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GroundstrokeAbilityInterface.generated.h"

class UCurveFloat;

UINTERFACE(MinimalAPI)
class UGroundstrokeAbilityInterface : public UInterface
{
	GENERATED_BODY()
};

class TENNISSTORY_API IGroundstrokeAbilityInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	float CalculateBallSpeed();
	
	UFUNCTION(BlueprintNativeEvent)
	int GetShotQuality();

	UFUNCTION(BlueprintNativeEvent)
	FName GetTrajectoryParamsRowName();
};
