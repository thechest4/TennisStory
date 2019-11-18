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

		TennisBall->Multicast_FollowPath(OwnerChar->BallAimingSplineComp, BallSpeed, TrajectoryCurve);

		TennisBall->LastPlayerToHit = OwnerChar;
	}
}

void UBallStrikingComponent::GenerateTrajectorySpline()
{
	if (!OwnerSplineComp)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("UBallStrikingComponent::GenerateTrajectorySpline - OwnerSplineComp was null!"));
		return;
	}

	if (!TrajectoryCurve)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("UBallStrikingComponent::GenerateTrajectorySpline - TrajectoryCurve was null!"));
		return;
	}

	FVector TargetLocation = OwnerTarget->GetActorLocation();

	OwnerSplineComp->SetWorldLocationAndRotation(OwnerChar->GetActorLocation(), OwnerChar->GetActorRotation());
	OwnerSplineComp->ClearSplinePoints();

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

	OwnerSplineComp->AddSplinePoint(ActorLoc, ESplineCoordinateSpace::World, false);
	OwnerSplineComp->SetTangentAtSplinePoint(0, StartTangent * 500.f, ESplineCoordinateSpace::World, false);

	MidPoint += FVector(0.f, 0.f, 200.f);

	OwnerSplineComp->AddSplinePoint(MidPoint, ESplineCoordinateSpace::World, false);
	
	OwnerSplineComp->AddSplinePoint(TargetLocation, ESplineCoordinateSpace::World, false);
	OwnerSplineComp->SetTangentAtSplinePoint(2, EndTangent * 500.f, ESplineCoordinateSpace::World, false);

	OwnerSplineComp->UpdateSpline();

	/*#include "DrawDebugHelpers.h"
	for (float i = 0.f; i < OwnerSplineComp->Duration; i += OwnerSplineComp->Duration / 15.f)
	{
		FVector SplineLoc = OwnerSplineComp->GetLocationAtTime(i, ESplineCoordinateSpace::World);

		DrawDebugSphere(GetWorld(), SplineLoc, 5.0f, 20, FColor::Purple, false, 3.f);
	}*/
}

FBallTrajectoryData UBallStrikingComponent::GetDataForCurrentSpline()
{
	if (!OwnerSplineComp)
	{
		return FBallTrajectoryData();
	}

	FBallTrajectoryData TrajectoryData = FBallTrajectoryData();

	for (int i = 0; i < OwnerSplineComp->GetNumberOfSplinePoints(); i++)
	{
		FVector Location = OwnerSplineComp->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
		FVector Tangent = OwnerSplineComp->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::World);
		TrajectoryData.AddTrajectoryPoint(Location, Tangent);
	}

	return TrajectoryData;
}

void UBallStrikingComponent::CopySplineFromData(FBallTrajectoryData TrajectoryData)
{
	if (!OwnerSplineComp)
	{
		return;
	}
	
	OwnerSplineComp->ClearSplinePoints();

	for (int i = 0; i < TrajectoryData.TrajectoryPoints.Num(); i++)
	{
		OwnerSplineComp->AddSplinePointAtIndex(TrajectoryData.TrajectoryPoints[i].Location, i, ESplineCoordinateSpace::World, false);
		OwnerSplineComp->SetTangentAtSplinePoint(i, TrajectoryData.TrajectoryPoints[i].Tangent, ESplineCoordinateSpace::World, false);
	}

	OwnerSplineComp->UpdateSpline();
}

float UBallStrikingComponent::CalculateChargedBallSpeed()
{
	float ChargeDuration = LastChargeEndTime - LastChargeStartTime;
	return FMath::Lerp(MinBallSpeed, MaxBallSpeed, FMath::Min(ChargeDuration / MaxChargeDuration, 1.0f));
}

void FBallTrajectoryData::AddTrajectoryPoint(FVector PointLocation, FVector PointTangent)
{
	FBallTrajectoryPoint TrajectoryPoint = FBallTrajectoryPoint(PointLocation, PointTangent);
	TrajectoryPoints.Add(TrajectoryPoint);
}
