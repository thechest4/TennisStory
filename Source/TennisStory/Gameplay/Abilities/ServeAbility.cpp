// Fill out your copyright notice in the Description page of Project Settings.

#include "ServeAbility.h"
#include "Gameplay/Abilities/Tasks/TS_AbilityTask_PlayMontageAndWait.h"
#include "TennisStoryGameState.h"
#include "TennisStoryGameMode.h"
#include "Gameplay/Ball/TennisBall.h"
#include "Gameplay/Ball/BallMovementComponent.h"
#include "Player/TennisStoryCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "../Ball/GlobalBallVelocityModifier.h"

UServeAbility::UServeAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bReplicateInputDirectly = true;

	bServeReleased = false;
}

bool UServeAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags /*= nullptr*/, const FGameplayTagContainer* TargetTags /*= nullptr*/, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/) const
{
	bool bSuperResult = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
	
	bool bHasBallAttached = false;
	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(ActorInfo->OwnerActor);
	if (OwnerChar)
	{
		bHasBallAttached = OwnerChar->HasBallAttached();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("UServeAbility::CanActivateAbility - Failed to get OwnerChar ref"));
	}

	return !CurrentMontageTask && bHasBallAttached && bSuperResult;
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

	if (OwnerChar->HasAuthority())
	{
		OwnerChar->Multicast_LockMovement();
	}

	if (OwnerChar->BallStrikingComp)
	{
		OwnerChar->BallStrikingComp->SetShotSourceAndFallbackTypeTags(ShotSourceTag, FallbackShotTypeTag);
	}

	//This needs to happen after the shot tags are set
	OwnerChar->EnablePlayerTargeting(ETargetingContext::Service, TennisBall);
}

void UServeAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (CurrentMontageTask)
	{
		CurrentMontageTask->OnBlendOut.RemoveDynamic(this, &UServeAbility::HandleServeMontageBlendOut);
		CurrentMontageTask = nullptr;
	}
	
	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(ActorInfo->OwnerActor);
	checkf(OwnerChar, TEXT("UServeAbility::EndAbility - Somehow OwnerChar is null!"))

	OwnerChar->OnPlayerHitServe().Remove(OnPlayerHitServeDelegateHandle);
	OnPlayerHitServeDelegateHandle.Reset();

	if (bServeReleased)
	{
		OwnerChar->Multicast_ExitServiceState();
		OwnerChar->UnclampLocation();
	}

	OwnerChar->DisablePlayerTargeting();
	
	if (OwnerChar->HasAuthority())
	{
		OwnerChar->Multicast_UnlockMovement();
	}

	if (OwnerChar->BallStrikingComp)
	{
		OwnerChar->BallStrikingComp->ResetAllShotTags();
	}
}

void UServeAbility::HandleServeMontageBlendOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UServeAbility::HandlePlayerHitServe(ATennisStoryCharacter* Player)
{
	ATennisStoryGameState* GameState = GetWorld()->GetGameState<ATennisStoryGameState>();
	ATennisBall* TennisBall = (GameState) ? GameState->GetTennisBall().Get() : nullptr;
	if (TennisBall)
	{
		int ServeQualityIndex = EvaluateServeQuality(TennisBall->BallMovementComp);
		
		ensureMsgf(OrderedSpeedMultipliers.Num() == static_cast<int>(EServeQuality::MAX), TEXT("UServeAbility::HandlePlayerHitServe - OrderedSpeedMultipliers doesn't have the right number of items!"));

		if (OrderedSpeedMultipliers.Num() != static_cast<int>(EServeQuality::MAX))
		{
			return;
		}

		ensureMsgf(OrderedServeHitVFX.Num() == static_cast<int>(EServeQuality::MAX), TEXT("UServeAbility::HandlePlayerHitServe - HitFX array doesn't have the right number of items!"));
		ensureMsgf(OrderedServeHitSFX.Num() == static_cast<int>(EServeQuality::MAX), TEXT("UServeAbility::HandlePlayerHitServe - HitSFX array doesn't have the right number of items!"));

		//This call changes the BallMovementComponent CurrentMovementState so it should be done before the FollowPath call
		TennisBall->InterruptServiceToss();

		if (Player->HasAuthority())
		{	
			//Tell the simulated proxy that the serve was hit, only if is currently the authority serving
			if (Player->IsLocallyControlled())
			{
				TennisBall->Multicast_InterruptServiceToss();
			}

			ATennisStoryGameMode* GameMode = GetWorld()->GetAuthGameMode<ATennisStoryGameMode>();

			checkf(GameMode, TEXT("UServeAbility::HandlePlayerHitServe - GameMode was null"))

			GameMode->DetermineHitLegality(Player);

			FTrajectoryParams TrajParams = UBallAimingFunctionLibrary::RetrieveTrajectoryParamsFromDataProvider(ShotSourceTag, FGameplayTagContainer::EmptyContainer, Player->BallStrikingComp->GetDesiredShotTypeTag(), FallbackShotTypeTag);

			FBallTrajectoryData TrajectoryData = UBallAimingFunctionLibrary::GenerateTrajectoryData(TrajParams, TennisBall->GetActorLocation(), Player->GetCurrentTargetLocation(), TennisBall);

			TArray<float> MultiplicativeVelocityModifiers = { OrderedSpeedMultipliers[ServeQualityIndex] };
			UGlobalBallVelocityModifier::CalculateGlobalVelocityModifiers(TrajectoryData, MultiplicativeVelocityModifiers);

			TrajectoryData.ApplyVelocityModifiers(MultiplicativeVelocityModifiers);

			EBoundsContext BoundsContextForServe = (GameState->GetServiceSide() == EServiceSide::Deuce) ? EBoundsContext::ServiceDeuce : EBoundsContext::ServiceAd;

			TennisBall->Multicast_FollowPath(TrajectoryData, BoundsContextForServe, Player);

			TennisBall->LastPlayerToHit = Player;
			TennisBall->bWasLastHitAServe = true;

			GameMode->ReportServeHit();
			
			if (OrderedServeHitVFX.Num() == static_cast<int>(EServeQuality::MAX))
			{
				UParticleSystem* HitVFX = OrderedServeHitVFX[ServeQualityIndex];
				if (HitVFX)
				{
					TennisBall->Multicast_SpawnHitParticleEffect(HitVFX, TennisBall->GetActorLocation());
				}
			}

			if (OrderedServeHitSFX.Num() == static_cast<int>(EServeQuality::MAX))
			{
				USoundBase* HitSFX = OrderedServeHitSFX[ServeQualityIndex];
				if (HitSFX)
				{
					Player->Multicast_PlaySound(HitSFX, TennisBall->GetActorLocation());
				}
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
