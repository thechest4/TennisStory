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
	bReplicateInputDirectly = true;

	bVolleyReleased = false;
	bCurrentShotIsForehand = false;
	bCurrentShotIsHigh = false;
}

bool UVolleyAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags /*= nullptr*/, const FGameplayTagContainer* TargetTags /*= nullptr*/, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/) const
{
	bool bSuperResult = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);

	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(ActorInfo->OwnerActor);
	bool bHasOwnerPermission = (OwnerChar) ? OwnerChar->DoesSwingAbilityHavePermissionToActivate(this) : false;

	return !CurrentMontageTask && bSuperResult && bHasOwnerPermission;
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

	bVolleyReleased = false;

	if (OwnerChar->BallStrikingComp)
	{
		OwnerChar->BallStrikingComp->OnBallHit().AddUObject(this, &UVolleyAbility::HandleBallHit);
		OwnerChar->BallStrikingComp->SetCurrentGroundstrokeAbility(this);
		OwnerChar->BallStrikingComp->SetShotSourceAndFallbackTypeTags(GetShotSourceTag(), GetFallbackShotTypeTag());
	}

	//Can't call UpdateShotContext until shot tags are set since it may trigger a call to get the trajectory params
	UpdateShotContext(TennisBall, OwnerChar);
	UAnimMontage* MontageToPlay = GetVolleyMontage();
	SetStrikeZonePosition(OwnerChar);

	CurrentMontageTask = UTS_AbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("PlayVolleyMontage"), MontageToPlay, 1.0f, TEXT("Wind Up"));
	CurrentMontageTask->OnBlendOut.AddDynamic(this, &UVolleyAbility::HandleVolleyMontageBlendOut);
	CurrentMontageTask->ReadyForActivation();
	
	CurrentTickingTask = UAbilityTask_Tick::CreateTask(this, FName(TEXT("Volley Context Check Task")));
	CurrentTickingTask->OnTaskTick().AddUObject(this, &UVolleyAbility::HandleTaskTick);
	CurrentTickingTask->ReadyForActivation();

	if (OwnerChar->HasAuthority())
	{
		OwnerChar->Multicast_ModifyBaseSpeed(BaseSpeedDuringAbility);
	}

	//This needs to happen after the shot tags are set
	OwnerChar->EnablePlayerTargeting(ETargetingContext::Volley);
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
		if (OwnerChar->BallStrikingComp)
		{
			OwnerChar->BallStrikingComp->StopBallStriking();
		}

		if (OwnerChar->IsLocallyControlled())
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
			OwnerChar->BallStrikingComp->SetCurrentGroundstrokeAbility(nullptr);

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

void UVolleyAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
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
	
	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(ActorInfo->OwnerActor);
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
	bool bPrevShotForehand = bCurrentShotIsForehand;
	bool bPrevShotHigh = bCurrentShotIsHigh;

	bCurrentShotIsForehand = OwnerCharacter->ShouldPerformForehand(TennisBall);
	bCurrentShotIsHigh = false;
	TWeakObjectPtr<const USplineComponent> BallSplineComp = TennisBall->GetSplineComponent();
	if (BallSplineComp.IsValid())
	{
		FVector FutureBallLocation = BallSplineComp->FindLocationClosestToWorldLocation(OwnerCharacter->GetActorLocation(), ESplineCoordinateSpace::World);
		const float MinHeightForHighVolley = 130.f;
		bCurrentShotIsHigh = FutureBallLocation.Z >= MinHeightForHighVolley;
	}

	if (bCurrentShotIsHigh != bPrevShotHigh)
	{
		FGameplayTagContainer ContextTags = FGameplayTagContainer((bCurrentShotIsHigh) ? FGameplayTag::RequestGameplayTag(TEXT("Shot.Context.Volley.High")) : FGameplayTag::RequestGameplayTag(TEXT("Shot.Context.Volley.Low")));
		OwnerCharacter->BallStrikingComp->SetShotContextTags(ContextTags);
	}

	return bCurrentShotIsForehand != bPrevShotForehand || bCurrentShotIsHigh != bPrevShotHigh;
}

UAnimMontage* UVolleyAbility::GetVolleyMontage()
{
	if (bCurrentShotIsForehand && bCurrentShotIsHigh)
	{
		return ForehandMontage_High;
	}
	else if (bCurrentShotIsForehand && !bCurrentShotIsHigh)
	{
		return ForehandMontage_Low;
	}
	else if (!bCurrentShotIsForehand && bCurrentShotIsHigh)
	{
		return BackhandMontage_High;
	}
	else if (!bCurrentShotIsForehand && !bCurrentShotIsHigh)
	{
		return BackhandMontage_Low;
	}

	checkNoEntry()

	return ForehandMontage_High;
}

void UVolleyAbility::SetStrikeZonePosition(ATennisStoryCharacter* OwnerCharacter)
{
	if (bCurrentShotIsForehand && bCurrentShotIsHigh)
	{
		OwnerCharacter->PositionStrikeZone(EStrokeType::Forehand_High);
	}
	else if (bCurrentShotIsForehand && !bCurrentShotIsHigh)
	{
		OwnerCharacter->PositionStrikeZone(EStrokeType::Forehand);
	}
	else if (!bCurrentShotIsForehand && bCurrentShotIsHigh)
	{
		OwnerCharacter->PositionStrikeZone(EStrokeType::Backhand_High);
	}
	else if (!bCurrentShotIsForehand && !bCurrentShotIsHigh)
	{
		OwnerCharacter->PositionStrikeZone(EStrokeType::Backhand);
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

		CurrentMontageTask = UTS_AbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("PlayVolleyMontage"), MontageToPlay, 1.0f, TEXT("Wind Up"));
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

