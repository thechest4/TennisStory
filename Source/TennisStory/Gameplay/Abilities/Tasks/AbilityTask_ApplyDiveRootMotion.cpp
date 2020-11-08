// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilityTask_ApplyDiveRootMotion.h"
#include "GameFramework/RootMotionSource.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Net/UnrealNetwork.h"
#include "Gameplay/Abilities/DiveAbility.h"

UAbilityTask_ApplyDiveRootMotion::UAbilityTask_ApplyDiveRootMotion()
	: DiveDirection(ForceInitToZero)
	, DiveDistance(0.f)
	, Duration(0.f)
	, PositionCurve(nullptr)
{
}

UAbilityTask_ApplyDiveRootMotion* UAbilityTask_ApplyDiveRootMotion::CreateTask(UGameplayAbility* OwningAbility, FName InstanceName, FVector Direction, float argDiveDistance, float DiveDuration, UCurveFloat* argPositionCurve)
{
	UAbilityTask_ApplyDiveRootMotion* NewTask = NewAbilityTask<UAbilityTask_ApplyDiveRootMotion>(OwningAbility, InstanceName);

	NewTask->DiveDirection = Direction;
	NewTask->DiveDistance = argDiveDistance;
	NewTask->Duration = DiveDuration;
	NewTask->PositionCurve = argPositionCurve;

	NewTask->SharedInitAndApply();

	return NewTask;
}

void UAbilityTask_ApplyDiveRootMotion::Activate()
{
	SetWaitingOnAvatar();
}

void UAbilityTask_ApplyDiveRootMotion::SharedInitAndApply()
{
	if (AbilitySystemComponent->AbilityActorInfo->MovementComponent.IsValid())
	{
		MovementComponent = Cast<UCharacterMovementComponent>(AbilitySystemComponent->AbilityActorInfo->MovementComponent.Get());

		if (MovementComponent)
		{
			ForceName = ForceName.IsNone() ? FName("AbilityTaskApplyDiveRootMotion") : ForceName;
			FRootMotionSource_DiveMotion* DiveMotion = new FRootMotionSource_DiveMotion();
			DiveMotion->InstanceName = ForceName;
			DiveMotion->AccumulateMode = ERootMotionAccumulateMode::Override;
			DiveMotion->Priority = 500;
			DiveMotion->DiveDirection = DiveDirection;
			DiveMotion->DiveDistance = DiveDistance;
			DiveMotion->Duration = Duration;
			DiveMotion->PositionCurve = PositionCurve;
			
			RootMotionSourceID = MovementComponent->ApplyRootMotionSource(DiveMotion);
		}
	}
}

void UAbilityTask_ApplyDiveRootMotion::Finish()
{
	bIsFinished = true;

	if (!bIsSimulating)
	{
		AActor* MyActor = GetAvatarActor();
		if (MyActor)
		{
			MyActor->ForceNetUpdate();
		}
	}

	EndTask();
}

void UAbilityTask_ApplyDiveRootMotion::PreDestroyFromReplication()
{
	Finish();
}

void UAbilityTask_ApplyDiveRootMotion::OnDestroy(bool AbilityIsEnding)
{
	if (MovementComponent)
	{
		MovementComponent->RemoveRootMotionSourceByID(RootMotionSourceID);
	}

	Super::OnDestroy(AbilityIsEnding);
}

void UAbilityTask_ApplyDiveRootMotion::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAbilityTask_ApplyDiveRootMotion, DiveDirection);
	DOREPLIFETIME(UAbilityTask_ApplyDiveRootMotion, DiveDistance);
	DOREPLIFETIME(UAbilityTask_ApplyDiveRootMotion, Duration);
	DOREPLIFETIME(UAbilityTask_ApplyDiveRootMotion, PositionCurve);
}
