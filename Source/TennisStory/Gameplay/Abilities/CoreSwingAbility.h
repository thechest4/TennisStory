// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CoreSwingAbility.generated.h"

UINTERFACE(MinimalAPI)
class UCoreSwingAbility : public UInterface
{
	GENERATED_BODY()
};

//CoreSwingAbilities are abilities that are activated by the Swing input based on context.  The activation and cancellation are controlled by the TennisStoryCharacter
class TENNISSTORY_API ICoreSwingAbility
{
	GENERATED_BODY()

public:
	virtual void ReleaseSwing() = 0;

	virtual bool HasReleased() = 0;
};
