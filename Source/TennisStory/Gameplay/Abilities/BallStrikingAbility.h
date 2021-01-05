// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTags.h"
#include "BallStrikingAbility.generated.h"

class UCurveFloat;

UINTERFACE(MinimalAPI)
class UBallStrikingAbility : public UInterface
{
	GENERATED_BODY()
};

class TENNISSTORY_API IBallStrikingAbility
{
	GENERATED_BODY()

public:
	virtual FGameplayTag GetShotSourceTag() = 0;

	virtual FGameplayTag GetFallbackShotTypeTag() = 0;

	virtual int GetShotQuality() { return 0; }

	virtual float GetSpeedMultiplier() { return 1.f; }
};
