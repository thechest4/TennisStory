// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerTargetActor.h"
#include "Gameplay/HalfCourt.h"
#include "Components/StaticMeshComponent.h"

APlayerTargetActor::APlayerTargetActor()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = TargetMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetMesh"));
	TargetMesh->SetVisibility(false);
	TargetMesh->SetGenerateOverlapEvents(false);
	TargetMesh->SetCollisionProfileName(TEXT("NoCollision"));

	bCurrentlyVisible = false;
}

void APlayerTargetActor::AddInputVector(FVector Direction, float Value)
{
	CurrentInputVector += Direction * Value;
}

void APlayerTargetActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bCurrentlyVisible)
	{
		FVector MovementVector = ConsumeCurrentInputVector();
		MovementVector.Normalize();

		if (GetWorld()->GetTimeSeconds() - TargetingStartedTime < LockedTargetingDuration)
		{
			ESnapPoint SnapPointToAimAt = ESnapPoint::Mid;

			if (MovementVector.Y > 0.f)
			{
				SnapPointToAimAt = ESnapPoint::Right;
			}
			else if (MovementVector.Y < -0.f)
			{
				SnapPointToAimAt = ESnapPoint::Left;
			}

			if (MovementVector.Y != 0.f)
			{
				LastSnapPoint = SnapPointToAimAt;
				SetActorLocation(CurrentTargetCourt->GetSnapPointLocation(SnapPointToAimAt));
			}
		}
		else
		{
			MovementVector = MovementVector * MoveSpeed * DeltaSeconds;

			if (MovementVector != FVector::ZeroVector)
			{
				FVector NewLocation = GetActorLocation() + MovementVector;
				CurrentTargetCourt->ClampLocationToCourtBounds(NewLocation);
				SetActorLocation(NewLocation);
			}
		}
	}
}

void APlayerTargetActor::ShowTargetOnCourt(TWeakObjectPtr<AHalfCourt> CourtToAimAt)
{
	if (CourtToAimAt.IsValid())
	{
		CurrentTargetCourt = CourtToAimAt;

		LastSnapPoint = ESnapPoint::Mid;
		SetActorLocation(CurrentTargetCourt->GetSnapPointLocation(ESnapPoint::Mid));

		bCurrentlyVisible = true;
		TargetingStartedTime = GetWorld()->GetTimeSeconds();

		TargetMesh->SetVisibility(true);

		CurrentInputVector = FVector::ZeroVector;
	}
}

void APlayerTargetActor::HideTarget()
{
	bCurrentlyVisible = false;

	TargetMesh->SetVisibility(false);
}

FVector APlayerTargetActor::ConsumeCurrentInputVector()
{
	FVector LastInputVector = CurrentInputVector;
	CurrentInputVector = FVector::ZeroVector;

	return LastInputVector;
}

