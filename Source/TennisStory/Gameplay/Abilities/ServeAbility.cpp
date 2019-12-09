// Fill out your copyright notice in the Description page of Project Settings.

#include "ServeAbility.h"
#include "Gameplay/Abilities/Tasks/TS_AbilityTask_PlayMontageAndWait.h"
#include "TennisStoryGameState.h"
#include "Gameplay/Ball/TennisBall.h"
#include "Gameplay/Ball/BallMovementComponent.h"
#include "Player/TennisStoryCharacter.h"
#include "Kismet/GameplayStatics.h"

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
		int ServeQualityIndex = EvaluateServeQuality(TennisBall->BallMovementComp);
		
		checkf(OrderedServeSpeeds.Num() == static_cast<int>(EServeQuality::MAX), TEXT("UServeAbility::HandlePlayerHitServe - ServeSpeeds array doesn't have the right number of items!"))
		checkf(OrderedServeHitFX.Num() == static_cast<int>(EServeQuality::MAX), TEXT("UServeAbility::HandlePlayerHitServe - HitFX array doesn't have the right number of items!"))

		//This call changes the BallMovementComponent CurrentMovementState so it should be done before the FollowPath call
		TennisBall->InterruptServiceToss();

		if (Player->HasAuthority() && ServeTrajectoryCurve)
		{
			FBallTrajectoryData TrajectoryData = UBallAimingFunctionLibrary::GenerateTrajectoryData(ServeTrajectoryCurve, TennisBall->GetActorLocation(), Player->GetCurrentTargetLocation(), 200.f, 500.f);
			
			float ServeSpeed = OrderedServeSpeeds[ServeQualityIndex];

			TennisBall->Multicast_FollowPath(TrajectoryData, ServeSpeed, true);

			TennisBall->LastPlayerToHit = Player;
			
			UParticleSystem* HitFX = OrderedServeHitFX[ServeQualityIndex];
			if (HitFX)
			{
				TennisBall->Multicast_SpawnHitParticleEffect(HitFX, TennisBall->GetActorLocation());
			}
		}
	}
}

int UServeAbility::EvaluateServeQuality(UBallMovementComponent* BallMovementComp)
{
	//NOTE(achester): Formula for getting the duration of the window for any serve evaluation:
	//		   TotalAlphaMargin = 2 * Margin
	//		      AlphaPerFrame	= DeltaTime / (TossDuration * 0.5)
	// WindowLength (in frames) = TotalAlphaMargin / AlphaPerFrame 

	const float TossAlpha = BallMovementComp->GetCurrentTossAlpha();
	
	//This value is how we judge the quality of the serve.  The Alpha values range from 0-2, with the perfect serve being at 1
	//This means a perfect serve will have an EvaluationValue close to 0.f (1 - 1), and a bad serve will have an EvaluationValue close to 1.f (0-1 or 2-1)
	float EvaluationValue = FMath::Abs(TossAlpha - 1.f);

	if (EvaluationValue <= 0.f + PerfectServeMargin)
	{
		return static_cast<int>(EServeQuality::Perfect);
	}
	else if (EvaluationValue >= 1.f - BadServeMargin)
	{
		return static_cast<int>(EServeQuality::Bad);
	}
	
	return static_cast<int>(EServeQuality::Normal);
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
