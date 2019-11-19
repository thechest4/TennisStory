// Fill out your copyright notice in the Description page of Project Settings.

#include "BallStrikingComponent.h"
#include "Player/TennisStoryCharacter.h"
#include "Player/PlayerTargetActor.h"
#include "Gameplay/TennisRacquet.h"
#include "Gameplay/Ball/BallAimingFunctionLibrary.h"
#include "Gameplay/Ball/TennisBall.h"
#include "Gameplay/Ball/BallMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/CapsuleComponent.h"

UBallStrikingComponent::UBallStrikingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	LastReceivedTrajectoryData = FBallTrajectoryData();
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
	if (OwnerChar->HasAuthority())
	{
		if (OwnerRacquet)
		{
			OwnerRacquet->OverlapDetectionComp->OnComponentBeginOverlap.AddDynamic(this, &UBallStrikingComponent::HandleRacquetOverlapBegin);
		}
	}
}

void UBallStrikingComponent::StopBallStriking()
{
	if (OwnerChar->HasAuthority())
	{
		if (OwnerRacquet)
		{
			OwnerRacquet->OverlapDetectionComp->OnComponentBeginOverlap.RemoveDynamic(this, &UBallStrikingComponent::HandleRacquetOverlapBegin);
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

		TennisBall->Multicast_FollowPath(GetLastTrajectoryData(), BallSpeed, true);

		TennisBall->LastPlayerToHit = OwnerChar;
	}
}

void UBallStrikingComponent::SetTrajectory(FBallTrajectoryData TrajectoryData)
{
	if (!OwnerSplineComp)
	{
		return;
	}

	UBallAimingFunctionLibrary::ApplyTrajectoryDataToSplineComp(TrajectoryData, OwnerSplineComp);

	LastReceivedTrajectoryData = TrajectoryData;
}

float UBallStrikingComponent::CalculateChargedBallSpeed()
{
	float ChargeDuration = LastChargeEndTime - LastChargeStartTime;
	return FMath::Lerp(MinBallSpeed, MaxBallSpeed, FMath::Min(ChargeDuration / MaxChargeDuration, 1.0f));
}
