// Fill out your copyright notice in the Description page of Project Settings.

#include "DiveAbility.h"
#include "Player/TennisStoryCharacter.h"
#include "Tasks/TS_AbilityTask_PlayMontageAndWait.h"
#include "Tasks/AbilityTask_Tick.h"

UDiveAbility::UDiveAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	CurrentDiveDirection = FVector::ZeroVector;
	DiveSpeed = 100.f;
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

	OwnerAnimInstance = (OwnerChar->GetMesh()) ? OwnerChar->GetMesh()->GetAnimInstance() : nullptr;
	if (!OwnerAnimInstance.IsValid())
	{
		return;
	}

	if (!CommitAbility(Handle, OwnerInfo, ActivationInfo))
	{
		return;
	}
	
	//Get dive direction from TargetData
	FVector2D InputVector = FVector2D::ZeroVector;

	const FGameplayAbilityTargetData* TargetData = (TriggerEventData) ? TriggerEventData->TargetData.Get(0) : nullptr;
	if (TargetData)
	{
		InputVector = FVector2D(TargetData->GetEndPoint());
	}

	//Calculate a worldspace direction from InputVector
	const FRotator OwnerControlRotation = OwnerChar->Controller->GetControlRotation();
	const FRotator YawRotation(0, OwnerControlRotation.Yaw, 0);
	
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	CurrentDiveDirection = (ForwardDirection * InputVector.Y + RightDirection * InputVector.X).GetSafeNormal();

	//If no input detected just go with the actor's forward vector
	if (CurrentDiveDirection == FVector::ZeroVector)
	{
		CurrentDiveDirection = OwnerChar->GetActorForwardVector();
	}

	OwnerChar->PositionStrikeZone(EStrokeType::Dive);

	CurrentMontageTask = UTS_AbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("PlayDiveMontage"), DiveMontage, 1.0f);
	CurrentMontageTask->OnBlendOut.AddDynamic(this, &UDiveAbility::HandleDiveMontageBlendOut);
	CurrentMontageTask->ReadyForActivation();
	
	CurrentTickingTask = UAbilityTask_Tick::CreateTask(this, FName(TEXT("Dive Ability Tick")));
	CurrentTickingTask->OnTaskTick().AddUObject(this, &UDiveAbility::HandleTaskTick);
	CurrentTickingTask->ReadyForActivation();

	if (OwnerChar->HasAuthority())
	{
		OwnerChar->Multicast_LockMovement();
	}

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
	
	if (CurrentTickingTask)
	{
		CurrentTickingTask->ExternalCancel();
		CurrentTickingTask->OnTaskTick().RemoveAll(this);
		CurrentTickingTask = nullptr;
	}

	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(CurrentActorInfo->OwnerActor);
	if (OwnerChar)
	{
		if (OwnerChar->IsLocallyControlled())
		{
			OwnerChar->DisablePlayerTargeting();
		}

		if (OwnerChar->HasAuthority())
		{
			OwnerChar->Multicast_UnlockMovement();
		}
		
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
	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(CurrentActorInfo->OwnerActor);

	if (OwnerChar && CurrentDiveDirection.Size() > 0.f)
	{
		float SpeedCurveVal = OwnerAnimInstance->GetCurveValue("DiveSpeedCurve");
		FVector Translation = CurrentDiveDirection * DiveSpeed * SpeedCurveVal * DeltaTime;

		OwnerChar->AddActorWorldOffset(Translation, true);
	}
}

bool FDiveAbilityTargetData::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	DiveInputVector.NetSerialize(Ar, Map, bOutSuccess);

	return true;
}
