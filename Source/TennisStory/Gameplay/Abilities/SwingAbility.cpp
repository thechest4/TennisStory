// Fill out your copyright notice in the Description page of Project Settings.


#include "SwingAbility.h"
#include "Gameplay/Abilities/Tasks/TS_AbilityTask_PlayMontageAndWait.h"
#include "Gameplay/Abilities/Tasks/AbilityTask_Tick.h"
#include "TennisStoryGameState.h"
#include "Gameplay/Ball/TennisBall.h"
#include "Player/TennisStoryCharacter.h"
#include "Player/Components/BallStrikingComponent.h"
#include "AnimNotifies/BallStrikingWindow.h"

USwingAbility::USwingAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bReplicateInputDirectly = true;

	bCurrentShotIsForehand = false;
	bSwingReleased = false;
}

bool USwingAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags /*= nullptr*/, const FGameplayTagContainer* TargetTags /*= nullptr*/, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/) const
{
	bool bSuperResult = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);

	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(ActorInfo->OwnerActor);
	bool bHasOwnerPermission = (OwnerChar) ? OwnerChar->DoesSwingAbilityHavePermissionToActivate(this) : false;

	return !CurrentMontageTask && bSuperResult && bHasOwnerPermission;
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

	UpdateShotContext(TennisBall, OwnerChar);
	MontageToPlay = GetSwingMontage();
	SetStrikeZonePosition(OwnerChar);

	CurrentMontageTask = UTS_AbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("PlaySwingMontage"), MontageToPlay, 1.0f, TEXT("Wind Up"));
	CurrentMontageTask->OnBlendOut.AddDynamic(this, &USwingAbility::HandleSwingMontageBlendOut);
	CurrentMontageTask->ReadyForActivation();
	
	CurrentTickingTask = UAbilityTask_Tick::CreateTask(this, FName(TEXT("Swing Context Check Task")));
	CurrentTickingTask->OnTaskTick().AddUObject(this, &USwingAbility::HandleTaskTick);
	CurrentTickingTask->ReadyForActivation();

	if (OwnerChar->HasAuthority())
	{
		LastChargeStartTime = GetWorld()->GetTimeSeconds();

		OwnerChar->Multicast_ModifyBaseSpeed(BaseSpeedDuringAbility);
	}

	if (OwnerChar->BallStrikingComp)
	{
		OwnerChar->BallStrikingComp->SetCurrentGroundstrokeAbility(this);
		OwnerChar->BallStrikingComp->SetShotSourceAndFallbackTypeTags(GetShotSourceTag(), GetFallbackShotTypeTag());
	}

	//This needs to happen after the shot tags are set
	OwnerChar->EnablePlayerTargeting(ETargetingContext::GroundStroke);
}

void USwingAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	if (CurrentMontageTask)
	{
		CurrentMontageTask->OnBlendOut.RemoveDynamic(this, &USwingAbility::HandleSwingMontageBlendOut);
		CurrentMontageTask = nullptr;
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
			OwnerChar->Multicast_RestoreBaseSpeed();
			OwnerChar->Multicast_UnlockMovement();
			OwnerChar->Multicast_UnlockAbilities();
		}
		
		if (OwnerChar->BallStrikingComp)
		{
			OwnerChar->BallStrikingComp->SetCurrentGroundstrokeAbility(nullptr);

			//If this ability has been canceled, we want to retain our DesiredShotTags so the canceling ability can use them (DiveAbility) but we never want to retain our context tags since they are unique to this ability
			OwnerChar->BallStrikingComp->ResetAllShotTags(true, !bWasCancelled);
		}
	}
}

void USwingAbility::HandleSwingMontageBlendOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

float USwingAbility::GetSpeedMultiplier()
{
	float ChargeDuration = LastChargeEndTime - LastChargeStartTime;
	return FMath::Lerp(MinSpeedMult, MaxSpeedMult, FMath::Min(ChargeDuration / MaxChargeDuration, 1.0f));
}

int USwingAbility::GetShotQuality()
{
	float ChargeDuration = LastChargeEndTime - LastChargeStartTime;
	float ChargeQuality = ChargeDuration / MaxChargeDuration;
				
	return (ChargeQuality > ChargeThresholdForMediumHitSFX) ? 1 : 0;
}

void USwingAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (bSwingReleased)
	{
		return;
	}

	bSwingReleased = true;

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
			LastChargeEndTime = GetWorld()->GetTimeSeconds();

			OwnerChar->Multicast_LockMovement();
			OwnerChar->Multicast_LockAbilities(); //Locking abilities to prevent dive canceling after releasing a swing

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
					OwnerChar->BallStrikingComp->OnTimingForgivesnessEnded().AddUObject(this, &USwingAbility::HandleSwingForgivenessEnded);
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

void USwingAbility::ReleaseForgiveness()
{
	if (CurrentMontageTask)
	{
		CurrentMontageTask->JumpToSection(TEXT("Swing"));
	}
}

bool USwingAbility::UpdateShotContext(ATennisBall* TennisBall, ATennisStoryCharacter* OwnerCharacter)
{
	bool bPrevShotForehand = bCurrentShotIsForehand;

	bCurrentShotIsForehand = OwnerCharacter->ShouldPerformForehand(TennisBall);

	return bCurrentShotIsForehand != bPrevShotForehand;
}

UAnimMontage* USwingAbility::GetSwingMontage()
{
	return (bCurrentShotIsForehand) ? ForehandMontage : BackhandMontage;
}

void USwingAbility::SetStrikeZonePosition(ATennisStoryCharacter* OwnerCharacter)
{
	if (bCurrentShotIsForehand)
	{
		OwnerCharacter->PositionStrikeZone(EStrokeType::Forehand);
	}
	else
	{
		OwnerCharacter->PositionStrikeZone(EStrokeType::Backhand);
	}
}

void USwingAbility::HandleTaskTick(float DeltaTime)
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
		
		UAnimMontage* MontageToPlay = GetSwingMontage();
		SetStrikeZonePosition(OwnerChar);

		CurrentMontageTask = UTS_AbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("PlaySwingMontage"), MontageToPlay, 1.0f, TEXT("Wind Up"));
		CurrentMontageTask->OnBlendOut.AddDynamic(this, &USwingAbility::HandleSwingMontageBlendOut);
		CurrentMontageTask->ReadyForActivation();
	}
}

void USwingAbility::HandleSwingForgivenessEnded()
{
	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(CurrentActorInfo->OwnerActor);
	if (OwnerChar && OwnerChar->BallStrikingComp)
	{
		OwnerChar->BallStrikingComp->OnTimingForgivesnessEnded().RemoveAll(this);

		OwnerChar->Multicast_ReleaseForgivingAbility(CurrentSpecHandle);
	}
}
