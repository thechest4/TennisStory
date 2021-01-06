// Fill out your copyright notice in the Description page of Project Settings.

#include "SmashAbility.h"
#include "Gameplay/Abilities/Tasks/TS_AbilityTask_PlayMontageAndWait.h"
#include "TennisStoryGameState.h"
#include "Gameplay/Ball/TennisBall.h"
#include "Player/TennisStoryCharacter.h"
#include "Player/Components/BallStrikingComponent.h"
#include "AnimNotifies/BallStrikingWindow.h"

USmashAbility::USmashAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	bSmashReleased = false;
}

bool USmashAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags /*= nullptr*/, const FGameplayTagContainer* TargetTags /*= nullptr*/, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/) const
{
	bool bSuperResult = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);

	return !CurrentMontageTask && bSuperResult;
}

void USmashAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* OwnerInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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

	bSmashReleased = false;

	OwnerChar->SetCurrentStance(ESwingStance::Neutral);
	SetStrikeZonePosition(OwnerChar);

	FName StartSection = bFromContextChange ? TEXT("Wind Up") : TEXT("Wind Up Loop");

	CurrentMontageTask = UTS_AbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("PlaySmashMontage"), SmashMontage, 1.0f, StartSection);
	CurrentMontageTask->OnBlendOut.AddDynamic(this, &USmashAbility::HandleSmashMontageBlendOut);
	CurrentMontageTask->ReadyForActivation();

	if (OwnerChar->HasAuthority())
	{
		OwnerChar->Multicast_ModifyBaseSpeed(BaseSpeedDuringAbility);
	}

	if (OwnerChar->BallStrikingComp)
	{
		OwnerChar->BallStrikingComp->SetCurrentBallStrikingAbility(this);
		OwnerChar->BallStrikingComp->SetShotSourceAndFallbackTypeTags(GetShotSourceTag(), GetFallbackShotTypeTag());
	}

	if (bFromContextChange)
	{
		//This needs to happen after the shot tags are set
		OwnerChar->EnablePlayerTargeting(ETargetingContext::GroundStroke);
	}
}

void USmashAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (CurrentMontageTask)
	{
		CurrentMontageTask->OnBlendOut.RemoveDynamic(this, &USmashAbility::HandleSmashMontageBlendOut);
		CurrentMontageTask = nullptr;
	}

	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(CurrentActorInfo->OwnerActor);
	if (OwnerChar)
	{
		OwnerChar->SetCurrentStance(ESwingStance::Neutral);

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
			OwnerChar->BallStrikingComp->SetCurrentBallStrikingAbility(nullptr);

			//If this ability has been canceled, we want to retain our DesiredShotTags so the canceling ability can use them (DiveAbility) but we never want to retain our context tags since they are unique to this ability
			OwnerChar->BallStrikingComp->ResetAllShotTags(true, !bWasCancelled);
		}
	}
}

void USmashAbility::HandleSmashMontageBlendOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void USmashAbility::ReleaseForgiveness()
{
	if (CurrentMontageTask)
	{
		CurrentMontageTask->JumpToSection(TEXT("Swing"));
	}
}

void USmashAbility::ReleaseSwing()
{
	if (bSmashReleased)
	{
		return;
	}

	bSmashReleased = true;

	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(CurrentActorInfo->OwnerActor);
	if (OwnerChar)
	{
		OwnerChar->DisablePlayerTargeting();

		if (OwnerChar->HasAuthority())
		{
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
					OwnerChar->BallStrikingComp->OnTimingForgivesnessEnded().AddUObject(this, &USmashAbility::HandleSwingForgivenessEnded);
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

void USmashAbility::SetStrikeZonePosition(ATennisStoryCharacter* OwnerCharacter)
{
	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(CurrentActorInfo->OwnerActor);
	OwnerCharacter->PositionStrikeZone(EStrikeZoneLocation::Smash);
}

void USmashAbility::HandleSwingForgivenessEnded()
{
	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(CurrentActorInfo->OwnerActor);
	if (OwnerChar && OwnerChar->BallStrikingComp)
	{
		OwnerChar->BallStrikingComp->OnTimingForgivesnessEnded().RemoveAll(this);

		OwnerChar->Multicast_ReleaseForgivingAbility(CurrentSpecHandle);
	}
}
