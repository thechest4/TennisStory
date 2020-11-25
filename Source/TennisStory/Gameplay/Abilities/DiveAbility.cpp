// Fill out your copyright notice in the Description page of Project Settings.

#include "DiveAbility.h"
#include "Player/TennisStoryCharacter.h"
#include "Player/Components/BallStrikingComponent.h"
#include "Tasks/TS_AbilityTask_PlayMontageAndWait.h"
#include "Tasks/AbilityTask_ApplyDiveRootMotion.h"

UDiveAbility::UDiveAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	DiveDistance = 650.f;
}

bool UDiveAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags /*= nullptr*/, const FGameplayTagContainer* TargetTags /*= nullptr*/, OUT FGameplayTagContainer* OptionalRelevantTags /*= nullptr*/) const
{
	bool bSuperResult = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);

	return !CurrentMontageTask && bSuperResult;
}

void UDiveAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* OwnerInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, OwnerInfo, ActivationInfo, TriggerEventData);

	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(OwnerInfo->OwnerActor);
	if (!OwnerChar)
	{
		return;
	}

	OwnerAnimInstance = (OwnerChar->GetMesh()) ? OwnerChar->GetMesh()->GetAnimInstance() : nullptr;
	if (!OwnerAnimInstance.IsValid())
	{
		return;
	}

	if (!CommitAbility(Handle, OwnerInfo, ActivationInfo))
	{
		return;
	}
	
	//Get dive direction from TargetData
	FVector2D InputVector = FVector2D::ZeroVector;

	const FGameplayAbilityTargetData* TargetData = (TriggerEventData) ? TriggerEventData->TargetData.Get(0) : nullptr;
	if (TargetData)
	{
		InputVector = FVector2D(TargetData->GetEndPoint());
	}

	//Calculate a worldspace direction from InputVector
	const FRotator OwnerControlRotation = OwnerChar->Controller->GetControlRotation();
	const FRotator YawRotation(0, OwnerControlRotation.Yaw, 0);
	
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	FVector DiveDirection = (ForwardDirection * InputVector.Y + RightDirection * InputVector.X).GetSafeNormal();

	//If no input detected just go with the actor's forward vector
	if (DiveDirection == FVector::ZeroVector)
	{
		DiveDirection = OwnerChar->GetActorForwardVector();
	}

	OwnerChar->PositionStrikeZone(EStrokeType::Dive);

	CachedPrevRotation = OwnerChar->GetActorRotation();
	OwnerChar->SetActorRotation(DiveDirection.ToOrientationRotator());

	if (OwnerChar->GetLocalRole() == ROLE_AutonomousProxy)
	{
		OwnerChar->bEnableRotationFix = false;
	}

	CurrentMontageTask = UTS_AbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("PlayDiveMontage"), DiveMontage, 1.0f);
	CurrentMontageTask->OnBlendOut.AddDynamic(this, &UDiveAbility::HandleDiveMontageBlendOut);
	CurrentMontageTask->ReadyForActivation();

	if (DivePositionCurve)
	{
		CurrentRootMotionTask = UAbilityTask_ApplyDiveRootMotion::CreateTask(this, FName(TEXT("Dive Root Motion Task")), DiveDirection, DiveDistance, DiveMontage->CalculateSequenceLength() - DiveMontage->GetDefaultBlendOutTime(), DivePositionCurve);
		CurrentRootMotionTask->ReadyForActivation();
	}

	if (OwnerChar->BallStrikingComp)
	{
		OwnerChar->BallStrikingComp->SetCurrentGroundstrokeAbility(this);
		OwnerChar->BallStrikingComp->SetShotSourceAndFallbackTypeTags(GetShotSourceTag(), GetFallbackShotTypeTag());
	}

	//This needs to happen after shot tags are set
	OwnerChar->EnablePlayerTargeting(ETargetingContext::GroundStroke);
}

void UDiveAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	if (CurrentMontageTask)
	{
		CurrentMontageTask->OnBlendOut.RemoveDynamic(this, &UDiveAbility::HandleDiveMontageBlendOut);
		CurrentMontageTask = nullptr;
	}

	if (CurrentRootMotionTask)
	{
		CurrentRootMotionTask->Finish();
		CurrentRootMotionTask = nullptr;
	}

	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(CurrentActorInfo->OwnerActor);
	if (OwnerChar)
	{
		if (OwnerChar->IsLocallyControlled())
		{
			OwnerChar->DisablePlayerTargeting();
		}
		
		if (OwnerChar->BallStrikingComp)
		{
			OwnerChar->BallStrikingComp->SetCurrentGroundstrokeAbility(nullptr);
			OwnerChar->BallStrikingComp->ResetAllShotTags();
		}

		OwnerChar->SetActorRotation(CachedPrevRotation);
		
		if (OwnerChar->GetLocalRole() == ROLE_AutonomousProxy)
		{
			OwnerChar->bEnableRotationFix = true;
		}
	}
}

void UDiveAbility::HandleDiveMontageBlendOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

bool FDiveAbilityTargetData::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	DiveInputVector.NetSerialize(Ar, Map, bOutSuccess);

	return true;
}

FRootMotionSource_DiveMotion::FRootMotionSource_DiveMotion()
	: DiveDirection(ForceInitToZero)
	, DiveDistance(0.f)
	, PositionCurve(nullptr)
{

}

FRootMotionSource* FRootMotionSource_DiveMotion::Clone() const
{
	FRootMotionSource_DiveMotion* CopyPtr = new FRootMotionSource_DiveMotion(*this);
	return CopyPtr;
}

bool FRootMotionSource_DiveMotion::Matches(const FRootMotionSource* Other) const
{
	if (!FRootMotionSource::Matches(Other))
	{
		return false;
	}

	//We can safely downcast because we called the Matches function so we know they are the same type
	const FRootMotionSource_DiveMotion* OtherCast = static_cast<const FRootMotionSource_DiveMotion*>(Other);

	return DiveDirection == OtherCast->DiveDirection &&
		DiveDistance == OtherCast->DiveDistance &&
		Duration == OtherCast->Duration &&
		PositionCurve == OtherCast->PositionCurve;
}

void FRootMotionSource_DiveMotion::PrepareRootMotion(float SimulationTime, float MovementTickTime, const ACharacter& Character, const UCharacterMovementComponent& MoveComponent)
{
	RootMotionParams.Clear();
	
	if (Duration > SMALL_NUMBER && MovementTickTime > SMALL_NUMBER && SimulationTime > SMALL_NUMBER)
	{
		float CurrentRelativeTime = GetTime() / Duration;
		float TargetRelativeTime = (GetTime() + SimulationTime) / Duration;

		float CurrentRelativeDiveDistance = 0.f;
		float TargetRelativeDiveDistance = 0.f;

		if (PositionCurve)
		{
			CurrentRelativeDiveDistance = PositionCurve->GetFloatValue(CurrentRelativeTime);
			TargetRelativeDiveDistance = PositionCurve->GetFloatValue(TargetRelativeTime);
		}

		FVector CurrentDivePosition = DiveDirection * CurrentRelativeDiveDistance * DiveDistance;
		FVector TargetDivePosition = DiveDirection * TargetRelativeDiveDistance * DiveDistance;

		//I believe this calculates a velocity by solving the equation Displacement = Speed * Time for Speed
		FVector Force = (TargetDivePosition - CurrentDivePosition) / MovementTickTime;

		FTransform MotionTransform(Force);
		RootMotionParams.Set(MotionTransform);
	}

	SetTime(GetTime() + SimulationTime);
}

bool FRootMotionSource_DiveMotion::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	if (!FRootMotionSource::NetSerialize(Ar, Map, bOutSuccess))
	{
		return false;
	}

	Ar << DiveDirection;
	Ar << DiveDistance;
	Ar << PositionCurve;

	bOutSuccess = true;
	return true;
}

UScriptStruct* FRootMotionSource_DiveMotion::GetScriptStruct() const
{
	return FRootMotionSource_DiveMotion::StaticStruct();
}

FString FRootMotionSource_DiveMotion::ToSimpleString() const
{
	return FString::Printf(TEXT("[ID:%u]FRootMotionSource_DiveMotion %s"), LocalID, *InstanceName.GetPlainNameString());
}

void FRootMotionSource_DiveMotion::AddReferencedObjects(class FReferenceCollector& Collector)
{
	FRootMotionSource::AddReferencedObjects(Collector);
}
