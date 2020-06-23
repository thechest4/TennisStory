// Fill out your copyright notice in the Description page of Project Settings.

#include "VolleyAbility.h"
#include "Gameplay/Abilities/Tasks/TS_AbilityTask_PlayMontageAndWait.h"
#include "Gameplay/Abilities/Tasks/AbilityTask_Tick.h"
#include "TennisStoryGameState.h"
#include "Gameplay/Ball/TennisBall.h"
#include "Player/TennisStoryCharacter.h"
#include "Player/Components/BallStrikingComponent.h"

UVolleyAbility::UVolleyAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bReplicateInputDirectly = true;

	bVolleyReleased = false;
	bIsHighVolley = false;
	CurrentVolleyType = EVolleyType::PassiveVolley;
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
	CurrentVolleyType = EVolleyType::PassiveVolley;
	UAnimMontage* MontageToPlay = ForehandMontage_High;

	bool bIsForehand = ShouldChooseForehand(TennisBall, OwnerChar);
	
	bIsHighVolley = false;
	TWeakObjectPtr<const USplineComponent> BallSplineComp = TennisBall->GetSplineComponent();
	if (BallSplineComp.IsValid())
	{
		FVector FutureBallLocation = BallSplineComp->FindLocationClosestToWorldLocation(OwnerChar->GetActorLocation(), ESplineCoordinateSpace::World);
		const float MinHeightForHighVolley = 100.f;
		bIsHighVolley = FutureBallLocation.Z >= MinHeightForHighVolley;
	}

	if (!bIsForehand)
	{
		if (bIsHighVolley)
		{
			MontageToPlay = BackhandMontage_High;
			OwnerChar->PositionStrikeZone(EStrokeType::Backhand_High);
		}
		else
		{
			MontageToPlay = BackhandMontage_Low;
			OwnerChar->PositionStrikeZone(EStrokeType::Backhand);
		}
		
	}
	else
	{
		if (bIsHighVolley)
		{
			MontageToPlay = ForehandMontage_High;
			OwnerChar->PositionStrikeZone(EStrokeType::Forehand_High);
		}
		else
		{
			MontageToPlay = ForehandMontage_Low;
			OwnerChar->PositionStrikeZone(EStrokeType::Forehand);
		}
	}

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

	OwnerChar->EnablePlayerTargeting(ETargetingContext::Volley);

	if (OwnerChar->IsLocallyControlled())
	{
		OwnerChar->StartDistanceVisualizationToBall();
	}

	if (OwnerChar->BallStrikingComp)
	{
		OwnerChar->BallStrikingComp->OnBallHit().AddUObject(this, &UVolleyAbility::HandleBallHit);
		OwnerChar->BallStrikingComp->SetCurrentGroundstrokeAbility(this);
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
		if (OwnerChar->BallStrikingComp)
		{
			OwnerChar->BallStrikingComp->StopBallStriking();
		}

		if (OwnerChar->IsLocallyControlled())
		{
			OwnerChar->DisablePlayerTargeting();
			OwnerChar->StopDistanceVisualization();
		}

		if (OwnerChar->HasAuthority())
		{
			OwnerChar->Multicast_RestoreBaseSpeed();
		}
		
		if (OwnerChar->BallStrikingComp)
		{
			OwnerChar->BallStrikingComp->OnBallHit().RemoveAll(this);
			OwnerChar->BallStrikingComp->SetCurrentGroundstrokeAbility(nullptr);
		}
	}
}

void UVolleyAbility::HandleVolleyMontageBlendOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

float UVolleyAbility::CalculateBallSpeed_Implementation()
{
	switch (CurrentVolleyType)
	{
		case EVolleyType::PassiveVolley:
		{
			return (bIsHighVolley) ? PassiveVolleySpeed_High : PassiveVolleySpeed_Low;
		}
		case EVolleyType::ActiveVolley:
		{
			return (bIsHighVolley) ? ActiveVolleySpeed_High : ActiveVolleySpeed_Low;
		}
	}

	checkNoEntry()

	return PassiveVolleySpeed_High;
}

UCurveFloat* UVolleyAbility::GetTrajectoryCurve_Implementation()
{
	return (bIsHighVolley) ? TrajectoryCurve_High : TrajectoryCurve_Low;
}

int UVolleyAbility::GetShotQuality_Implementation()
{
	switch (CurrentVolleyType)
	{
		case EVolleyType::PassiveVolley:
		{
			return 0;
		}
		case EVolleyType::ActiveVolley:
		{
			return 1;
		}
	}

	checkNoEntry()

	return 0;
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

void UVolleyAbility::HandleTaskTick()
{
	GEngine->AddOnScreenDebugMessage(0, 0.01f, FColor::Green, TEXT("VolleyAbility::HandleTaskTick"));
}

