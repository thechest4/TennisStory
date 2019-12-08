// Fill out your copyright notice in the Description page of Project Settings.

#include "ServeAbility.h"
#include "Gameplay/Abilities/Tasks/TS_AbilityTask_PlayMontageAndWait.h"
#include "TennisStoryGameState.h"
#include "Gameplay/Ball/TennisBall.h"
#include "Player/TennisStoryCharacter.h"

UServeAbility::UServeAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bServeReleased(false)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bReplicateInputDirectly = true;
}

bool UServeAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags /*= nullptr*/, const FGameplayTagContainer* TargetTags /*= nullptr*/, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/) const
{
	bool bSuperResult = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);

	return !CurrentMontageTask && bSuperResult;
}

void UServeAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* OwnerInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, OwnerInfo, ActivationInfo, TriggerEventData);

	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(OwnerInfo->OwnerActor);
	if (!OwnerChar)
	{
		return;
	}

	OnPlayerHitServeDelegateHandle = OwnerChar->OnPlayerHitServe().AddUObject(this, &UServeAbility::HandlePlayerHitServe);

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

	bServeReleased = false;

	CurrentMontageTask = UTS_AbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("PlayServeMontage"), ServeMontage, 1.0f, TEXT("Toss"));
	CurrentMontageTask->OnBlendOut.AddDynamic(this, &UServeAbility::HandleServeMontageBlendOut);
	CurrentMontageTask->ReadyForActivation();
	
	//TODO(achester): Lock player targeting to the service box
	OwnerChar->EnablePlayerTargeting();
}

void UServeAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(ActorInfo->OwnerActor);
	checkf(OwnerChar, TEXT("UServeAbility::EndAbility - Somehow OwnerChar is null!"))

	OwnerChar->OnPlayerHitServe().Remove(OnPlayerHitServeDelegateHandle);
	OnPlayerHitServeDelegateHandle.Reset();
}

void UServeAbility::HandleServeMontageBlendOut()
{
	CurrentMontageTask->OnBlendOut.RemoveDynamic(this, &UServeAbility::HandleServeMontageBlendOut);
	CurrentMontageTask = nullptr;

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UServeAbility::HandlePlayerHitServe(ATennisStoryCharacter* Player)
{
	ATennisStoryGameState* GameState = GetWorld()->GetGameState<ATennisStoryGameState>();
	ATennisBall* TennisBall = (GameState) ? GameState->GetTennisBall().Get() : nullptr;
	if (TennisBall)
	{
		TennisBall->InterruptServiceToss();

		if (Player->HasAuthority() && ServeTrajectoryCurve)
		{
			FBallTrajectoryData TrajectoryData = UBallAimingFunctionLibrary::GenerateTrajectoryData(ServeTrajectoryCurve, TennisBall->GetActorLocation(), Player->GetCurrentTargetLocation(), 200.f, 500.f);

			TennisBall->Multicast_FollowPath(TrajectoryData, 1500.f, true);

			TennisBall->LastPlayerToHit = Player;
		}
	}
}

void UServeAbility::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (bServeReleased)
	{
		return;
	}

	if (CurrentMontageTask)
	{
		CurrentMontageTask->JumpToSection(TEXT("Swing"));
		bServeReleased = true;
	}

	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(ActorInfo->OwnerActor);
	if (OwnerChar)
	{
		OwnerChar->DisablePlayerTargeting();
	}
}
