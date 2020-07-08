// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotion_Base.h"
#include "AbilityTask_ApplyDiveRootMotion.generated.h"

UCLASS()
class TENNISSTORY_API UAbilityTask_ApplyDiveRootMotion : public UAbilityTask_ApplyRootMotion_Base
{
	GENERATED_BODY()
	
public:
	UAbilityTask_ApplyDiveRootMotion();

	static UAbilityTask_ApplyDiveRootMotion* CreateTask(UGameplayAbility* OwningAbility, FName InstanceName, FVector Direction, float DiveDistance, float DiveDuration, UCurveFloat* PositionCurve);

	virtual void Activate() override;
	
	void Finish();

	virtual void PreDestroyFromReplication() override;
	virtual void OnDestroy(bool AbilityIsEnding) override;

	UPROPERTY(Replicated)
	FVector DiveDirection;

	UPROPERTY(Replicated)
	float DiveDistance;
	
	UPROPERTY(Replicated)
	float Duration;

	UPROPERTY(Replicated)
	UCurveFloat* PositionCurve;

protected:
	virtual void SharedInitAndApply() override;
};
