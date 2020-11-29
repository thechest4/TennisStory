// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ForgivingAbilityInterface.generated.h"

UINTERFACE(MinimalAPI)
class UForgivingAbilityInterface : public UInterface
{
	GENERATED_BODY()
};

class TENNISSTORY_API IForgivingAbilityInterface
{
	GENERATED_BODY()

public:
	virtual void ReleaseForgiveness() = 0;
};
