// Fill out your copyright notice in the Description page of Project Settings.

#include "BallStrikingComponent.h"
#include "Player/TennisStoryCharacter.h"
#include "Player/PlayerTargetActor.h"
#include "Gameplay/TennisRacquet.h"
#include "Gameplay/Ball/BallAimingFunctionLibrary.h"
#include "Gameplay/Ball/TennisBall.h"
#include "Gameplay/Ball/BallMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/BoxComponent.h"
#include "TennisStoryGameMode.h"

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

void UBallStrikingComponent::SetChargeStartTime()
{
	LastChargeStartTime = GetWorld()->GetTimeSeconds();
}

void UBallStrikingComponent::SetChargeEndTime()
{
	LastChargeEndTime = GetWorld()->GetTimeSeconds();
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

		float BallSpeed = CalculateChargedBallSpeed();

		FBallTrajectoryData TrajectoryData = UBallAimingFunctionLibrary::GenerateTrajectoryData(GetTrajectoryCurve(), TennisBall->GetActorLocation(), OwnerTarget->GetActorLocation(), 200.f, 500.f);

		TennisBall->Multicast_FollowPath(TrajectoryData, BallSpeed, true, EBoundsContext::FullCourt);

		BallHitEvent.Broadcast();

		if (OwnerChar->HasAuthority())
		{
			TennisBall->LastPlayerToHit = OwnerChar;
			TennisBall->bWasLastHitAServe = false;

			if (HitFX)
			{
				TennisBall->Multicast_SpawnHitParticleEffect(HitFX, TennisBall->GetActorLocation());
			}

			ensureMsgf(OrderedHitSFX.Num() == 2, TEXT("UBallStrikingComponent::HandleRacquetOverlapBegin - OrderedHitSFX had the wrong number of items!"));
			if (OrderedHitSFX.Num() == 2)
			{
				float ChargeDuration = LastChargeEndTime - LastChargeStartTime;
				float ChargeQuality = ChargeDuration / MaxChargeDuration;
				
				int SFXIndex = (ChargeQuality > ThresholdForMediumHit) ? 1 : 0;

				OwnerChar->Multicast_PlaySound(OrderedHitSFX[SFXIndex], TennisBall->GetActorLocation());
			}
		}
	}
}

float UBallStrikingComponent::CalculateChargedBallSpeed()
{
	float ChargeDuration = LastChargeEndTime - LastChargeStartTime;
	return FMath::Lerp(MinBallSpeed, MaxBallSpeed, FMath::Min(ChargeDuration / MaxChargeDuration, 1.0f));
}
