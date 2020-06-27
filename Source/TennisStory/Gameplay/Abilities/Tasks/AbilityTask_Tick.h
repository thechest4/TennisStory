// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_Tick.generated.h"

DECLARE_EVENT_OneParam(UAbilityTask_Tick, FTaskTickEvent, float)

//Simple task that exposes a tick delegate
UCLASS()
class TENNISSTORY_API UAbilityTask_Tick : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAbilityTask_Tick();

	static UAbilityTask_Tick* CreateTask(UGameplayAbility* OwningAbility, FName InstanceName);

	FTaskTickEvent& OnTaskTick() { return TaskTickEvent; }

protected:
	virtual void TickTask(float DeltaTime) override;

	FTaskTickEvent TaskTickEvent;
};
