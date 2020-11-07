// Fill out your copyright notice in the Description page of Project Settings.

#include "BallStrikingComponent.h"
#include "Player/TennisStoryCharacter.h"
#include "Player/PlayerTargetActor.h"
#include "Gameplay/TennisRacquet.h"
#include "Gameplay/Ball/BallAimingFunctionLibrary.h"
#include "Gameplay/Ball/TennisBall.h"
#include "Gameplay/Ball/BallMovementComponent.h"
#include "Gameplay/Abilities/GroundstrokeAbilityInterface.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/BoxComponent.h"
#include "TennisStoryGameMode.h"
#include <../Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/Abilities/GameplayAbility.h>

UBallStrikingComponent::UBallStrikingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bBallStrikingAllowed = false;
}

void UBallStrikingComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerChar = Cast<ATennisStoryCharacter>(GetOwner());
	OwnerRacquet = (OwnerChar) ? OwnerChar->RacquetActor : nullptr;
	OwnerTarget = (OwnerChar) ? OwnerChar->TargetActor : nullptr;
	OwnerSplineComp = (OwnerChar) ? OwnerChar->BallAimingSplineComp : nullptr;
	
	checkf((OwnerRacquet || !OwnerTarget || !OwnerSplineComp), TEXT("UBallStrikingComponent::BeginPlay - Some required owner pointer not valid!"))
}

void UBallStrikingComponent::AllowBallStriking()
{
	if (bBallStrikingAllowed)
	{
		return;
	}

	//OwnerChar CAN be null when joining a session, as the animations (and notifies) can be replicated before BeginPlay has been called
	if (OwnerChar && OwnerChar->HasAuthority())
	{
		UBoxComponent* StrikeZone = OwnerChar->GetStrikeZone();
		if (StrikeZone)
		{
			StrikeZone->OnComponentBeginOverlap.AddDynamic(this, &UBallStrikingComponent::HandleRacquetOverlapBegin);
			StrikeZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

			bBallStrikingAllowed = true;
		}
	}
}

void UBallStrikingComponent::StopBallStriking()
{
	if (!bBallStrikingAllowed)
	{
		return;
	}

	if (OwnerChar && OwnerChar->HasAuthority())
	{
		UBoxComponent* StrikeZone = OwnerChar->GetStrikeZone();
		if (StrikeZone)
		{
			StrikeZone->OnComponentBeginOverlap.RemoveDynamic(this, &UBallStrikingComponent::HandleRacquetOverlapBegin);
			StrikeZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			bBallStrikingAllowed = false;
		}
	}
}

void UBallStrikingComponent::SetCurrentGroundstrokeAbility(UGameplayAbility* AbilityPtr)
{
	ensureMsgf(!AbilityPtr || (AbilityPtr && AbilityPtr->GetClass()->ImplementsInterface(UGroundstrokeAbilityInterface::StaticClass())), TEXT("Groundstroke ability did not implement GroundstrokeAbilityInterface!"));

	CurrentGroundstrokeAbility.SetObject(AbilityPtr);
}

void UBallStrikingComponent::HandleRacquetOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ATennisBall* TennisBall = Cast<ATennisBall>(OtherActor);
	if (OwnerTarget && TennisBall && TennisBall->GetCurrentBallState() != ETennisBallState::ServiceState)
	{
		if (TennisBall->LastPlayerToHit.IsValid() && TennisBall->LastPlayerToHit == OwnerChar)
		{
			return;
		}

		if (OwnerChar->HasAuthority())
		{
			ATennisStoryGameMode* GameMode = GetWorld()->GetAuthGameMode<ATennisStoryGameMode>();

			checkf(GameMode, TEXT("UBallStrikingComponent::HandleRacquetOverlapBegin - GameMode was null"))

			GameMode->DetermineHitLegality(OwnerChar);
		}

		UObject* GroundstrokeAbilityObj = CurrentGroundstrokeAbility.GetObject();

		ensureMsgf(GroundstrokeAbilityObj, TEXT("Invalid groundstroke ability object - no provided ball speed or trajectory"));
		
		if (OwnerChar->HasAuthority())
		{
			TennisBall->LastPlayerToHit = OwnerChar;
			TennisBall->bWasLastHitAServe = false;

			if (HitFX)
			{
				TennisBall->Multicast_SpawnHitParticleEffect(HitFX, TennisBall->GetActorLocation());
			}

			int ShotQuality = IGroundstrokeAbilityInterface::Execute_GetShotQuality(GroundstrokeAbilityObj);

			int SFXIndex = (ShotQuality >= 0 && ShotQuality < OrderedHitSFX.Num()) ? ShotQuality : 0;
			if (OrderedHitSFX.Num() > 0)
			{
				OwnerChar->Multicast_PlaySound(OrderedHitSFX[SFXIndex], TennisBall->GetActorLocation());
			}
		}

		float BallSpeed = IGroundstrokeAbilityInterface::Execute_CalculateBallSpeed(GroundstrokeAbilityObj);
		FName TrajParamsRowName = IGroundstrokeAbilityInterface::Execute_GetTrajectoryParamsRowName(GroundstrokeAbilityObj);

		FBallTrajectoryData TrajectoryData = UBallAimingFunctionLibrary::GenerateTrajectoryData(TrajParamsRowName, TennisBall->GetActorLocation(), OwnerTarget->GetActorLocation());

		TennisBall->Multicast_FollowPath(TrajectoryData, BallSpeed, EBoundsContext::FullCourt, OwnerChar);

		BallHitEvent.Broadcast();
	}
}
