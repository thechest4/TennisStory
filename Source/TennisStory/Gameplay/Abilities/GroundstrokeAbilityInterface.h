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
	virtual FGameplayTag GetShotSourceTag() = 0;

	virtual FGameplayTag GetFallbackShotTypeTag() = 0;

	virtual int GetShotQuality() { return 0; }

	virtual float GetSpeedMultiplier() { return 1.f; }
};
