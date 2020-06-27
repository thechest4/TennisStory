// Fill out your copyright notice in the Description page of Project Settings.

#include "DiveAbility.h"
#include "Player/TennisStoryCharacter.h"
#include "Tasks/TS_AbilityTask_PlayMontageAndWait.h"
#include "Tasks/AbilityTask_Tick.h"

UDiveAbility::UDiveAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UDiveAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags /*= nullptr*/, const FGameplayTagContainer* TargetTags /*= nullptr*/, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/) const
{
	bool bSuperResult = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);

	return !CurrentMontageTask && bSuperResult;
}

void UDiveAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* OwnerInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, OwnerInfo, ActivationInfo, TriggerEventData);

	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(OwnerInfo->OwnerActor);
	if (!OwnerChar)
	{
		return;
	}

	if (!CommitAbility(Handle, OwnerInfo, ActivationInfo))
	{
		return;
	}

	//Position Strike Zone

	CurrentMontageTask = UTS_AbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("PlayDiveMontage"), DiveMontage, 1.0f);
	CurrentMontageTask->OnBlendOut.AddDynamic(this, &UDiveAbility::HandleDiveMontageBlendOut);
	CurrentMontageTask->ReadyForActivation();
	
	CurrentTickingTask = UAbilityTask_Tick::CreateTask(this, FName(TEXT("Dive Ability Tick")));
	CurrentTickingTask->OnTaskTick().AddUObject(this, &UDiveAbility::HandleTaskTick);
	CurrentTickingTask->ReadyForActivation();

	/*if (OwnerChar->HasAuthority())
	{
		LastChargeStartTime = GetWorld()->GetTimeSeconds();

		OwnerChar->Multicast_ModifyBaseSpeed(BaseSpeedDuringAbility);
	}*/

	OwnerChar->EnablePlayerTargeting(ETargetingContext::GroundStroke);

	if (OwnerChar->BallStrikingComp)
	{
		OwnerChar->BallStrikingComp->SetCurrentGroundstrokeAbility(this);
	}
}

void UDiveAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	if (CurrentMontageTask)
	{
		CurrentMontageTask->OnBlendOut.RemoveDynamic(this, &UDiveAbility::HandleDiveMontageBlendOut);
		CurrentMontageTask = nullptr;
	}

	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(CurrentActorInfo->OwnerActor);
	if (OwnerChar)
	{
		if (OwnerChar->IsLocallyControlled())
		{
			OwnerChar->DisablePlayerTargeting();
		}

		/*if (OwnerChar->HasAuthority())
		{
			OwnerChar->Multicast_RestoreBaseSpeed();
		}*/
		
		if (OwnerChar->BallStrikingComp)
		{
			OwnerChar->BallStrikingComp->SetCurrentGroundstrokeAbility(nullptr);
		}
	}
}

void UDiveAbility::HandleDiveMontageBlendOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UDiveAbility::HandleTaskTick(float DeltaTime)
{
	//Update movement during roll
}
