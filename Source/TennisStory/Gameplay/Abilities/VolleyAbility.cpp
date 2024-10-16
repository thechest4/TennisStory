// Fill out your copyright notice in the Description page of Project Settings.

#include "VolleyAbility.h"
#include "Gameplay/Abilities/Tasks/TS_AbilityTask_PlayMontageAndWait.h"
#include "Gameplay/Abilities/Tasks/AbilityTask_Tick.h"
#include "TennisStoryGameState.h"
#include "Gameplay/Ball/TennisBall.h"
#include "Player/TennisStoryCharacter.h"
#include "Player/Components/BallStrikingComponent.h"
#include "AnimNotifies/BallStrikingWindow.h"

UVolleyAbility::UVolleyAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	bVolleyReleased = false;
	bCurrentShotIsHigh = false;
}

bool UVolleyAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags /*= nullptr*/, const FGameplayTagContainer* TargetTags /*= nullptr*/, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/) const
{
	bool bSuperResult = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);

	return !CurrentMontageTask && bSuperResult;
}

void UVolleyAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* OwnerInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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

	bool bFromContextChange = !TriggerEventData->InstigatorTags.HasTagExact(FGameplayTag::RequestGameplayTag(ATennisStoryCharacter::TAG_CORESWING_CONTEXTCHANGE));

	bVolleyReleased = false;

	if (OwnerChar->BallStrikingComp)
	{
		OwnerChar->BallStrikingComp->OnBallHit().AddUObject(this, &UVolleyAbility::HandleBallHit);
		OwnerChar->BallStrikingComp->SetCurrentBallStrikingAbility(this);
		OwnerChar->BallStrikingComp->SetShotSourceAndFallbackTypeTags(GetShotSourceTag(), GetFallbackShotTypeTag());
	}

	//Can't call UpdateShotContext until shot tags are set since it may trigger a call to get the trajectory params
	OwnerChar->SetCurrentStance(ESwingStance::Neutral);
	UpdateShotContext(TennisBall, OwnerChar);
	UAnimMontage* MontageToPlay = GetVolleyMontage();
	SetStrikeZonePosition(OwnerChar);

	FName StartSection = bFromContextChange ? TEXT("Wind Up") : TEXT("Wind Up Loop");

	CurrentMontageTask = UTS_AbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("PlayVolleyMontage"), MontageToPlay, 1.0f, StartSection);
	CurrentMontageTask->OnBlendOut.AddDynamic(this, &UVolleyAbility::HandleVolleyMontageBlendOut);
	CurrentMontageTask->ReadyForActivation();
	
	CurrentTickingTask = UAbilityTask_Tick::CreateTask(this, FName(TEXT("Volley Context Check Task")));
	CurrentTickingTask->OnTaskTick().AddUObject(this, &UVolleyAbility::HandleTaskTick);
	CurrentTickingTask->ReadyForActivation();

	if (OwnerChar->HasAuthority())
	{
		OwnerChar->Multicast_ModifyBaseSpeed(BaseSpeedDuringAbility);
	}

	if (bFromContextChange)
	{
		//This needs to happen after the shot tags are set
		OwnerChar->EnablePlayerTargeting(ETargetingContext::Volley);
	}
}

void UVolleyAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	if (CurrentMontageTask)
	{
		CurrentMontageTask->OnBlendOut.RemoveDynamic(this, &UVolleyAbility::HandleVolleyMontageBlendOut);
		CurrentMontageTask = nullptr;
	}

	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(CurrentActorInfo->OwnerActor);
	if (OwnerChar)
	{
		OwnerChar->SetCurrentStance(ESwingStance::Neutral);

		if (OwnerChar->BallStrikingComp)
		{
			OwnerChar->BallStrikingComp->StopBallStriking();
		}

		if (OwnerChar->IsLocallyControlled() && !bWasCancelled)
		{
			OwnerChar->DisablePlayerTargeting();
		}

		if (OwnerChar->HasAuthority())
		{
			OwnerChar->Multicast_RestoreBaseSpeed();
			OwnerChar->Multicast_UnlockMovement();
			OwnerChar->Multicast_UnlockAbilities();
		}
		
		if (OwnerChar->BallStrikingComp)
		{
			OwnerChar->BallStrikingComp->OnBallHit().RemoveAll(this);
			OwnerChar->BallStrikingComp->SetCurrentBallStrikingAbility(nullptr);

			//If this ability has been canceled, we want to retain our DesiredShotTags so the canceling ability can use them (DiveAbility) but we never want to retain our context tags since they are unique to this ability
			OwnerChar->BallStrikingComp->ResetAllShotTags(true, !bWasCancelled);
		}
	}
}

void UVolleyAbility::HandleVolleyMontageBlendOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UVolleyAbility::ReleaseForgiveness()
{
	if (CurrentMontageTask)
	{
		CurrentMontageTask->JumpToSection(TEXT("Swing"));
	}
}

void UVolleyAbility::ReleaseSwing()
{
	if (bVolleyReleased)
	{
		return;
	}

	bVolleyReleased = true;

	if (CurrentTickingTask)
	{
		CurrentTickingTask->ExternalCancel();
		CurrentTickingTask->OnTaskTick().RemoveAll(this);
		CurrentTickingTask = nullptr;
	}

	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(CurrentActorInfo->OwnerActor);
	if (OwnerChar)
	{
		OwnerChar->DisablePlayerTargeting();

		if (OwnerChar->HasAuthority())
		{
			OwnerChar->Multicast_LockMovement();
			OwnerChar->Multicast_LockAbilities();

			//Find the BallStrikingWindow so we can determine how much time the animation needs
			int SwingSectionIndex = CurrentMontage->GetSectionIndex(TEXT("Swing"));

			float StartTime;
			float EndTime;
			CurrentMontage->GetSectionStartAndEndTime(SwingSectionIndex, StartTime, EndTime);

			TArray<FAnimNotifyEventReference> AnimNotifies;
			CurrentMontage->GetAnimNotifiesFromDeltaPositions(StartTime, EndTime, AnimNotifies);

			const FAnimNotifyEvent* BallStrikingWindowNotify = nullptr;
			for (int i = 0; i < AnimNotifies.Num(); i++)
			{
				if (AnimNotifies[i].GetNotify()->NotifyStateClass->IsA<UBallStrikingWindow>())
				{
					BallStrikingWindowNotify = AnimNotifies[i].GetNotify();
					break;
				}
			}

			if (BallStrikingWindowNotify)
			{
				float TriggerTime = BallStrikingWindowNotify->GetTriggerTime() - StartTime;

				if (OwnerChar->BallStrikingComp && OwnerChar->BallStrikingComp->ShouldWaitForTimingForgiveness(TriggerTime))
				{
					//Bind to forgiveness end handler
					OwnerChar->BallStrikingComp->OnTimingForgivesnessEnded().AddUObject(this, &UVolleyAbility::HandleSwingForgivenessEnded);
				}
				else
				{
					HandleSwingForgivenessEnded();
				}
			}
			else
			{
				HandleSwingForgivenessEnded();
			}
		}
	}
}

bool UVolleyAbility::UpdateShotContext(ATennisBall* TennisBall, ATennisStoryCharacter* OwnerCharacter)
{
	ESwingStance PrevStance = OwnerCharacter->GetCurrentStance();
	bool bPrevShotHigh = bCurrentShotIsHigh;

	ESwingStance NewStance = OwnerCharacter->CalculateNewSwingStance(TennisBall);
	OwnerCharacter->SetCurrentStance(NewStance);

	bCurrentShotIsHigh = false;
	
	bool bIsInFrontQuarter = false;
	ATennisStoryGameState* GameState = GetWorld()->GetGameState<ATennisStoryGameState>();
	TWeakObjectPtr<AHalfCourt> PlayerCourt = (GameState) ? GameState->GetCourtForCharacter(OwnerCharacter) : nullptr;
	if (PlayerCourt.IsValid())
	{
		bIsInFrontQuarter = PlayerCourt->IsLocationInFrontQuarterOfCourt(OwnerCharacter->GetActorLocation());
	}

	TWeakObjectPtr<const USplineComponent> BallSplineComp = TennisBall->GetSplineComponent();
	if (BallSplineComp.IsValid())
	{
		FVector FutureBallLocation = BallSplineComp->FindLocationClosestToWorldLocation(OwnerCharacter->GetActorLocation(), ESplineCoordinateSpace::World);
		const float MinHeightForHighVolley = 140.f;
		bCurrentShotIsHigh = FutureBallLocation.Z >= MinHeightForHighVolley && bIsInFrontQuarter;
	}

	if (OwnerCharacter->BallStrikingComp->GetShotContextTags().Num() == 0 || bCurrentShotIsHigh != bPrevShotHigh)
	{
		FGameplayTagContainer ContextTags = FGameplayTagContainer((bCurrentShotIsHigh) ? FGameplayTag::RequestGameplayTag(TEXT("Shot.Context.Volley.High")) : FGameplayTag::RequestGameplayTag(TEXT("Shot.Context.Volley.Low")));
		OwnerCharacter->BallStrikingComp->SetShotContextTags(ContextTags);
	}

	return PrevStance != NewStance || bCurrentShotIsHigh != bPrevShotHigh;
}

UAnimMontage* UVolleyAbility::GetVolleyMontage()
{
	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(CurrentActorInfo->OwnerActor);

	if (OwnerChar->GetCurrentStance() == ESwingStance::Forehand && bCurrentShotIsHigh)
	{
		return ForehandMontage_High;
	}
	else if (OwnerChar->GetCurrentStance() == ESwingStance::Forehand && !bCurrentShotIsHigh)
	{
		return ForehandMontage_Low;
	}
	else if (OwnerChar->GetCurrentStance() == ESwingStance::Backhand && bCurrentShotIsHigh)
	{
		return BackhandMontage_High;
	}
	else if (OwnerChar->GetCurrentStance() == ESwingStance::Backhand && !bCurrentShotIsHigh)
	{
		return BackhandMontage_Low;
	}

	checkNoEntry()

	return ForehandMontage_High;
}

void UVolleyAbility::SetStrikeZonePosition(ATennisStoryCharacter* OwnerCharacter)
{
	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(CurrentActorInfo->OwnerActor);

	if (OwnerChar->GetCurrentStance() == ESwingStance::Forehand  && bCurrentShotIsHigh)
	{
		OwnerCharacter->PositionStrikeZone(EStrikeZoneLocation::Forehand_High);
	}
	else if (OwnerChar->GetCurrentStance() == ESwingStance::Forehand && !bCurrentShotIsHigh)
	{
		OwnerCharacter->PositionStrikeZone(EStrikeZoneLocation::Forehand);
	}
	else if (OwnerChar->GetCurrentStance() == ESwingStance::Backhand && bCurrentShotIsHigh)
	{
		OwnerCharacter->PositionStrikeZone(EStrikeZoneLocation::Backhand_High);
	}
	else if (OwnerChar->GetCurrentStance() == ESwingStance::Backhand && !bCurrentShotIsHigh)
	{
		OwnerCharacter->PositionStrikeZone(EStrikeZoneLocation::Backhand);
	}
}

void UVolleyAbility::HandleBallHit()
{
	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(CurrentActorInfo->OwnerActor);
	if (OwnerChar)
	{
		if (OwnerChar->BallStrikingComp)
		{
			OwnerChar->BallStrikingComp->OnBallHit().RemoveAll(this);
		}

		if (CurrentMontageTask)
		{
			OwnerChar->DisablePlayerTargeting();
			OwnerChar->BallStrikingComp->StopBallStriking();
		}
	}
}

void UVolleyAbility::HandleTaskTick(float DeltaTime)
{
	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(CurrentActorInfo->OwnerActor);
	
	ATennisStoryGameState* GameState = GetWorld()->GetGameState<ATennisStoryGameState>();
	ATennisBall* TennisBall = (GameState) ? GameState->GetTennisBall().Get() : nullptr;

	ensureMsgf(OwnerChar && TennisBall, TEXT("OwnerChar or TennisBall was null"));

	if (!OwnerChar || !TennisBall)
	{
		return;
	}

	bool bShotContextChanged = UpdateShotContext(TennisBall, OwnerChar);

	if (bShotContextChanged)
	{
		CurrentMontageTask->OnBlendOut.RemoveAll(this);
		
		UAnimMontage* MontageToPlay = GetVolleyMontage();
		SetStrikeZonePosition(OwnerChar);

		CurrentMontageTask = UTS_AbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("PlayVolleyMontage"), MontageToPlay, 1.0f, TEXT("Wind Up Loop"));
		CurrentMontageTask->OnBlendOut.AddDynamic(this, &UVolleyAbility::HandleVolleyMontageBlendOut);
		CurrentMontageTask->ReadyForActivation();
	}
}

void UVolleyAbility::HandleSwingForgivenessEnded()
{
	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(CurrentActorInfo->OwnerActor);
	if (OwnerChar && OwnerChar->BallStrikingComp)
	{
		OwnerChar->BallStrikingComp->OnTimingForgivesnessEnded().RemoveAll(this);

		OwnerChar->Multicast_ReleaseForgivingAbility(CurrentSpecHandle);
	}
}

