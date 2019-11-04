// Fill out your copyright notice in the Description page of Project Settings.


#include "SwingAbility.h"
#include "Gameplay/Abilities/Tasks/TS_AbilityTask_PlayMontageAndWait.h"
#include "TennisStoryGameState.h"
#include "Gameplay/TennisBall.h"
#include "Player/TennisStoryCharacter.h"
#include "Player/Components/BallStrikingComponent.h"

USwingAbility::USwingAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bSwingReleased(false)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bReplicateInputDirectly = true;
}

bool USwingAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags /*= nullptr*/, const FGameplayTagContainer* TargetTags /*= nullptr*/, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/) const
{
	bool bSuperResult = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);

	return !CurrentMontageTask && bSuperResult;
}

void USwingAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* OwnerInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, OwnerInfo, ActivationInfo, TriggerEventData);

	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(OwnerInfo->OwnerActor);
	if (!OwnerChar)
	{
		return;
	}

	ATennisStoryGameState* GameState = GetWorld()->GetGameState<ATennisStoryGameState>();
	ATennisBall* TennisBall = (GameState) ? GameState->GetTennisBall().Get() : nullptr;
	if (!TennisBall)
	{
		return;
	}

	if (!CommitAbility(Handle, OwnerInfo, ActivationInfo))
	{
		return;
	}

	bSwingReleased = false;
	UAnimMontage* MontageToPlay = ForehandMontage;

	{
		FVector DirToBall = TennisBall->GetActorLocation() - OwnerChar->GetActorLocation();

		float DotProd = FVector::DotProduct(DirToBall.GetSafeNormal(), OwnerChar->GetAimRightVector());

		if (DotProd < 0.0f)
		{
			MontageToPlay = BackhandMontage;
		}
	}

	CurrentMontageTask = UTS_AbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("PlaySwingMontage"), MontageToPlay, 1.0f, TEXT("Wind Up"));
	CurrentMontageTask->OnBlendOut.AddDynamic(this, &USwingAbility::HandleSwingMontageBlendOut);
	CurrentMontageTask->ReadyForActivation();

	OwnerChar->BallStrikingComp->SetChargeStartTime();

	OwnerChar->EnablePlayerTargeting();
}

void USwingAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void USwingAbility::HandleSwingMontageBlendOut(/*UAnimMontage* AnimMontage, bool bInterrupted*/)
{
	CurrentMontageTask->OnBlendOut.RemoveDynamic(this, &USwingAbility::HandleSwingMontageBlendOut);
	CurrentMontageTask = nullptr;

	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(CurrentActorInfo->OwnerActor);
	if (OwnerChar)
	{
		OwnerChar->DisablePlayerTargeting();
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void USwingAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (bSwingReleased)
	{
		return;
	}

	if (CurrentMontageTask)
	{
		CurrentMontageTask->JumpToSection(TEXT("Swing"));
		bSwingReleased = true;
	}

	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(ActorInfo->OwnerActor);
	if (OwnerChar)
	{
		OwnerChar->BallStrikingComp->SetChargeEndTime();
		OwnerChar->FreezePlayerTarget();
	}
}
