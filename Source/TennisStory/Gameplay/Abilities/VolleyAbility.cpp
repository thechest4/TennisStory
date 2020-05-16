// Fill out your copyright notice in the Description page of Project Settings.

#include "VolleyAbility.h"
#include "Gameplay/Abilities/Tasks/TS_AbilityTask_PlayMontageAndWait.h"
#include "TennisStoryGameState.h"
#include "Gameplay/Ball/TennisBall.h"
#include "Player/TennisStoryCharacter.h"
#include "Player/Components/BallStrikingComponent.h"

UVolleyAbility::UVolleyAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bReplicateInputDirectly = true;

	bVolleyReleased = false;
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
	UAnimMontage* MontageToPlay = ForehandMontage;

	bool bIsForehand = ShouldChooseForehand(TennisBall, OwnerChar);
	if (!bIsForehand)
	{
		MontageToPlay = BackhandMontage;
		OwnerChar->PositionStrikeZone(EStrokeType::Backhand);
	}
	else
	{
		OwnerChar->PositionStrikeZone(EStrokeType::Forehand);
	}

	CurrentMontageTask = UTS_AbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("PlaySwingMontage"), MontageToPlay, 1.0f, TEXT("Wind Up"));
	CurrentMontageTask->OnBlendOut.AddDynamic(this, &UVolleyAbility::HandleVolleyMontageBlendOut);
	CurrentMontageTask->ReadyForActivation();

	if (OwnerChar->HasAuthority())
	{
		OwnerChar->BallStrikingComp->SetChargeStartTime();
		OwnerChar->Multicast_ModifyBaseSpeed(BaseSpeedDuringAbility);
	}

	OwnerChar->EnablePlayerTargeting(ETargetingContext::GroundStroke);

	if (OwnerChar->IsLocallyControlled())
	{
		OwnerChar->StartDistanceVisualizationToBall();
	}

	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Volley ability"));
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
		if (OwnerChar->IsLocallyControlled())
		{
			OwnerChar->DisablePlayerTargeting();
			OwnerChar->StopDistanceVisualization();
		}

		if (OwnerChar->HasAuthority())
		{
			OwnerChar->Multicast_RestoreBaseSpeed();
		}
	}
}

void UVolleyAbility::HandleVolleyMontageBlendOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UVolleyAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (bVolleyReleased)
	{
		return;
	}

	if (CurrentMontageTask)
	{
		CurrentMontageTask->JumpToSection(TEXT("Swing"));
		bVolleyReleased = true;
	}

	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(ActorInfo->OwnerActor);
	if (OwnerChar)
	{
		OwnerChar->DisablePlayerTargeting();

		if (OwnerChar->HasAuthority())
		{
			OwnerChar->BallStrikingComp->SetChargeEndTime();
		}
	}
}

bool UVolleyAbility::ShouldChooseForehand(ATennisBall* TennisBall, ATennisStoryCharacter* OwnerCharacter)
{
	FVector BallDirection = TennisBall->GetCurrentDirection();
	float DistanceToBall = FVector::Dist(TennisBall->GetActorLocation(), OwnerCharacter->GetActorLocation());

	FVector ProjectedBallLocation = TennisBall->GetActorLocation() + BallDirection * DistanceToBall;

	FVector DirToBallProjection = ProjectedBallLocation - OwnerCharacter->GetActorLocation();
	float DotProd = FVector::DotProduct(DirToBallProjection.GetSafeNormal(), OwnerCharacter->GetAimRightVector());
	
	return DotProd >= 0.f;
}


