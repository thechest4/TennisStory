// Fill out your copyright notice in the Description page of Project Settings.

#include "BallStrikingComponent.h"
#include "Player/TennisStoryCharacter.h"
#include "Player/PlayerTargetActor.h"
#include "Gameplay/TennisRacquet.h"
#include "Gameplay/Ball/TennisBall.h"
#include "Gameplay/Ball/BallMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SplineComponent.h"

#include "DrawDebugHelpers.h"

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
	
	if (!OwnerRacquet || !OwnerTarget)
	{	
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("UBallStrikingComponent::BeginPlay - OwnerRacquet or OwnerTarget was null!"));
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

		GenerateTrajectorySpline();

		UBallMovementComponent* BallMovementComp = TennisBall->FindComponentByClass<UBallMovementComponent>();
		if (BallMovementComp)
		{
			BallMovementComp->FollowPath(OwnerChar->BallAimingSplineComp, BallSpeed, TrajectoryCurve);
		}
		
		TennisBall->LastPlayerToHit = OwnerChar;
	}
}

void UBallStrikingComponent::GenerateTrajectorySpline()
{
	USplineComponent* SplineComp = (OwnerChar) ? OwnerChar->BallAimingSplineComp : nullptr;
	if (!SplineComp)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("UBallStrikingComponent::GenerateTrajectorySpline - SplineComp was null!"));
		return;
	}

	if (!TrajectoryCurve)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("UBallStrikingComponent::GenerateTrajectorySpline - TrajectoryCurve was null!"));
		return;
	}

	FVector TargetLocation = OwnerTarget->GetActorLocation();

	SplineComp->SetWorldLocationAndRotation(OwnerChar->GetActorLocation(), OwnerChar->GetActorRotation());
	SplineComp->ClearSplinePoints();

	FVector ActorLoc = OwnerChar->GetActorLocation();
	FVector MidPoint = (TargetLocation - ActorLoc) / 2.f + ActorLoc;
	
	FVector DirectionVec = TargetLocation - ActorLoc;
	DirectionVec.Z = 0.f;
	DirectionVec.Normalize();

	FRichCurve CurveData = TrajectoryCurve->FloatCurve;
	float LeaveSlope = CurveData.GetFirstKey().LeaveTangent;
	float ArriveSlope = CurveData.GetLastKey().ArriveTangent;

	float LeaveAngle = FMath::RadiansToDegrees(FMath::Atan(LeaveSlope));
	float ArriveAngle = FMath::RadiansToDegrees(FMath::Atan(ArriveSlope));

	FVector RightVec = FVector::CrossProduct(FVector::UpVector, DirectionVec);
	FVector StartTangent = DirectionVec.RotateAngleAxis(-LeaveAngle, RightVec);

	FVector EndTangent = DirectionVec.RotateAngleAxis(-ArriveAngle, RightVec);

	SplineComp->AddSplinePoint(ActorLoc, ESplineCoordinateSpace::World, false);
	SplineComp->SetTangentAtSplinePoint(0, StartTangent * 500.f, ESplineCoordinateSpace::World, false);

	MidPoint += FVector(0.f, 0.f, 200.f);

	SplineComp->AddSplinePoint(MidPoint, ESplineCoordinateSpace::World, false);
	
	SplineComp->AddSplinePoint(TargetLocation, ESplineCoordinateSpace::World, false);
	SplineComp->SetTangentAtSplinePoint(2, EndTangent * 500.f, ESplineCoordinateSpace::World, false);

	SplineComp->UpdateSpline();

	for (float i = 0.f; i < SplineComp->Duration; i += SplineComp->Duration / 15.f)
	{
		FVector SplineLoc = SplineComp->GetLocationAtTime(i, ESplineCoordinateSpace::World);

		DrawDebugSphere(GetWorld(), SplineLoc, 5.0f, 20, FColor::Purple, false, 3.f);
	}
}

float UBallStrikingComponent::CalculateChargedBallSpeed()
{
	float ChargeDuration = LastChargeEndTime - LastChargeStartTime;
	return FMath::Lerp(MinBallSpeed, MaxBallSpeed, FMath::Min(ChargeDuration / MaxChargeDuration, 1.0f));
}
