// Fill out your copyright notice in the Description page of Project Settings.


#include "SwingAbility.h"
#include "Gameplay/Abilities/Tasks/TS_AbilityTask_PlayMontageAndWait.h"
#include "TennisStoryGameMode.h"
#include "Gameplay/TennisBall.h"
#include "Player/TennisStoryCharacter.h"
#include "Player/Components/BallStrikingComponent.h"

void USwingAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* OwnerInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, OwnerInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, OwnerInfo, ActivationInfo))
	{
		return;
	}

	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(OwnerInfo->OwnerActor);
	if (!OwnerChar)
	{
		return;
	}

	ATennisStoryGameMode* GameMode = GetWorld()->GetAuthGameMode<ATennisStoryGameMode>();
	ATennisBall* TennisBall = (GameMode) ? GameMode->GetTennisBall().Get() : nullptr;
	if (!TennisBall)
	{
		return;
	}

	UAnimMontage* MontageToPlay = ForehandMontage;

	{
		FVector DirToBall = TennisBall->GetActorLocation() - OwnerChar->GetActorLocation();

		float DotProd = FVector::DotProduct(DirToBall.GetSafeNormal(), FVector::RightVector);

		if (DotProd < 0.0f)
		{
			MontageToPlay = BackhandMontage;
		}
	}

	CurrentMontageTask = UTS_AbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("PlaySwingMontage"), MontageToPlay, 1.0f, TEXT("Wind Up"));
	CurrentMontageTask->OnBlendOut.AddDynamic(this, &USwingAbility::HandleSwingMontageBlendOut);
	CurrentMontageTask->ReadyForActivation();

	OwnerChar->BallStrikingComp->SetChargeStartTime();

	OwnerChar->EnablePlayerTargeting();
}

void USwingAbility::HandleSwingMontageBlendOut(/*UAnimMontage* AnimMontage, bool bInterrupted*/)
{
	CurrentMontageTask->OnBlendOut.RemoveDynamic(this, &USwingAbility::HandleSwingMontageBlendOut);
	CurrentMontageTask = nullptr;

	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(CurrentActorInfo->OwnerActor);
	if (OwnerChar)
	{
		OwnerChar->DisablePlayerTargeting();
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void USwingAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (CurrentMontageTask)
	{
		CurrentMontageTask->JumpToSection(TEXT("Swing"));
	}

	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(ActorInfo->OwnerActor);
	if (OwnerChar)
	{
		OwnerChar->BallStrikingComp->SetChargeEndTime();
		OwnerChar->FreezePlayerTarget();
	}
}
