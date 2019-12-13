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

UBallStrikingComponent::UBallStrikingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBallStrikingComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerChar = Cast<ATennisStoryCharacter>(GetOwner());
	OwnerRacquet = (OwnerChar) ? OwnerChar->RacquetActor : nullptr;
	OwnerTarget = (OwnerChar) ? OwnerChar->TargetActor : nullptr;
	OwnerSplineComp = (OwnerChar) ? OwnerChar->BallAimingSplineComp : nullptr;
	
	if (!OwnerRacquet || !OwnerTarget || !OwnerSplineComp)
	{	
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("UBallStrikingComponent::BeginPlay - Some required owner pointer not valid!"));
	}
}

void UBallStrikingComponent::AllowBallStriking()
{
	//OwnerChar CAN be null when joining a session, as the animations (and notifies) can be replicated before BeginPlay has been called
	if (OwnerChar && OwnerChar->HasAuthority())
	{
		UBoxComponent* StrikeZone = OwnerChar->GetStrikeZone();
		if (StrikeZone)
		{
			StrikeZone->OnComponentBeginOverlap.AddDynamic(this, &UBallStrikingComponent::HandleRacquetOverlapBegin);
			StrikeZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
	}
}

void UBallStrikingComponent::StopBallStriking()
{
	if (OwnerChar && OwnerChar->HasAuthority())
	{
		UBoxComponent* StrikeZone = OwnerChar->GetStrikeZone();
		if (StrikeZone)
		{
			StrikeZone->OnComponentBeginOverlap.RemoveDynamic(this, &UBallStrikingComponent::HandleRacquetOverlapBegin);
			StrikeZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
	if (OwnerTarget && TennisBall)
	{
		if (TennisBall->LastPlayerToHit.IsValid() && TennisBall->LastPlayerToHit == OwnerChar)
		{
			return;
		}

		float BallSpeed = CalculateChargedBallSpeed();

		FBallTrajectoryData TrajectoryData = UBallAimingFunctionLibrary::GenerateTrajectoryData(GetTrajectoryCurve(), TennisBall->GetActorLocation(), OwnerTarget->GetActorLocation(), 200.f, 500.f);

		TennisBall->Multicast_FollowPath(TrajectoryData, BallSpeed, true, EBoundsContext::FullCourt);

		TennisBall->LastPlayerToHit = OwnerChar;
	}
}

float UBallStrikingComponent::CalculateChargedBallSpeed()
{
	float ChargeDuration = LastChargeEndTime - LastChargeStartTime;
	return FMath::Lerp(MinBallSpeed, MaxBallSpeed, FMath::Min(ChargeDuration / MaxChargeDuration, 1.0f));
}
