// Fill out your copyright notice in the Description page of Project Settings.

#include "BallStrikingComponent.h"
#include "Player/TennisStoryCharacter.h"
#include "Player/PlayerTargetActor.h"
#include "Gameplay/TennisRacquet.h"
#include "Gameplay/TennisBall.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

UBallStrikingComponent::UBallStrikingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBallStrikingComponent::AllowBallStriking()
{
	ATennisStoryCharacter* OwnerCharacter = Cast<ATennisStoryCharacter>(GetOwner());
	ATennisRacquet* OwnerRacquet = (OwnerCharacter) ? OwnerCharacter->RacquetActor : nullptr;
	if (OwnerRacquet)
	{
		OwnerRacquet->OverlapDetectionComp->OnComponentBeginOverlap.AddDynamic(this, &UBallStrikingComponent::HandleRacquetOverlapBegin);
	}
}

void UBallStrikingComponent::StopBallStriking()
{
	ATennisStoryCharacter* OwnerCharacter = Cast<ATennisStoryCharacter>(GetOwner());
	ATennisRacquet* OwnerRacquet = (OwnerCharacter) ? OwnerCharacter->RacquetActor : nullptr;
	if (OwnerRacquet)
	{
		OwnerRacquet->OverlapDetectionComp->OnComponentBeginOverlap.RemoveDynamic(this, &UBallStrikingComponent::HandleRacquetOverlapBegin);
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
	ATennisStoryCharacter* OwnerCharacter = Cast<ATennisStoryCharacter>(GetOwner());
	APlayerTargetActor* PlayerTarget = (OwnerCharacter) ? OwnerCharacter->TargetActor : nullptr;

	ATennisBall* TennisBall = Cast<ATennisBall>(OtherActor);
	if (PlayerTarget && TennisBall)
	{
		float BallSpeed = CalculateChargedBallSpeed();

		//The math expects a positive gravity, but the ProjMovementComponent returns a negative one.  So multiply by -1
		float Gravity = -1 * TennisBall->ProjMovementComp->GetGravityZ();
		FRotator BallRotation = GetTrajectoryRotation(TennisBall->GetActorLocation(), PlayerTarget->GetActorLocation(), BallSpeed, Gravity);
		TennisBall->SetActorRotation(BallRotation);
		TennisBall->ProjMovementComp->SetVelocityInLocalSpace(BallSpeed * FVector(1.0f, 0.0f, 0.0f));
	}
}

FRotator UBallStrikingComponent::GetTrajectoryRotation(FVector BallLocation, FVector TargetLocation, float DesiredSpeed, float Gravity)
{
	// Projectile trajectory derivation from: https://blog.forrestthewoods.com/solving-ballistic-trajectories-b0165523348c

	FVector TargetRelativeLocation = TargetLocation - BallLocation;

	FVector DirectionVector = TargetRelativeLocation;
	DirectionVector.Z = 0.0f; //zero out the pitch so that the trajectory is relative to the xy plane, not the direction vector
	DirectionVector.Normalize();

	//Get the distances to the target, represented in 2 axes
	float HorzDistance = TargetRelativeLocation.Size2D();
	float VertDistance = TargetRelativeLocation.Z;

	float ThetaRads = FMath::DegreesToRadians(45.0f);

	float Sqrt_term = FMath::Pow(DesiredSpeed, 4) - Gravity * (Gravity * FMath::Pow(HorzDistance, 2) + 2 * FMath::Pow(DesiredSpeed, 2) * VertDistance);
	if (Sqrt_term >= 0.0f) //if sqrt is negative that indicates that the trajectory is undefined (out of range or otherwise incalculable).  So fall back to the max range theta value of 45 degrees
	{
		float Top_term = FMath::Pow(DesiredSpeed, 2) - FMath::Sqrt(Sqrt_term);
		float Bottom_term = Gravity * HorzDistance;
		ThetaRads = FMath::Atan(Top_term / Bottom_term);
	}

	//Calculate final launch direction by applying theta to the direction vector
	FVector RelativeLaunchDirection(FMath::Cos(ThetaRads), 0.0f, FMath::Sin(ThetaRads));
	FVector LaunchDirection = FRotationMatrix::MakeFromXZ(DirectionVector, FVector::UpVector).TransformVector(RelativeLaunchDirection);

	return LaunchDirection.Rotation();
}

float UBallStrikingComponent::CalculateChargedBallSpeed()
{
	float ChargeDuration = LastChargeEndTime - LastChargeStartTime;
	return FMath::Lerp(MinBallSpeed, MaxBallSpeed, FMath::Min(ChargeDuration / MaxChargeDuration, 1.0f));
}
