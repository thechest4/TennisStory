// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilityTask_Tick.h"

UAbilityTask_Tick::UAbilityTask_Tick()
{
	bTickingTask = true;
}

UAbilityTask_Tick* UAbilityTask_Tick::CreateTask(UGameplayAbility* OwningAbility, FName InstanceName)
{
	return NewAbilityTask<UAbilityTask_Tick>(OwningAbility, InstanceName);
}

void UAbilityTask_Tick::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	TaskTickEvent.Broadcast();
}
