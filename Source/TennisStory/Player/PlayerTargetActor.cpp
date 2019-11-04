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

	if (bCurrentlyVisible && bCurrentlyMovable)
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
			else if (MovementVector.Y < 0.f)
			{
				SnapPointToAimAt = ESnapPoint::Left;
			}

			if (MovementVector.Y != 0.f)
			{
				FVector AimVector = GetOwnerControlRotationVector();

				LastSnapPoint = SnapPointToAimAt;
				SetActorLocation(CurrentTargetCourt->GetSnapPointLocation(AimVector, SnapPointToAimAt));
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

void APlayerTargetActor::ShowTargetOnCourt(TWeakObjectPtr<AHalfCourt> CourtToAimAt, bool bShowTarget)
{
	if (CourtToAimAt.IsValid())
	{
		CurrentTargetCourt = CourtToAimAt;

		FVector AimVector = GetOwnerControlRotationVector();

		LastSnapPoint = ESnapPoint::Mid;
		SetActorLocation(CurrentTargetCourt->GetSnapPointLocation(AimVector, ESnapPoint::Mid));

		bCurrentlyVisible = true;
		bCurrentlyMovable = true;
		TargetingStartedTime = GetWorld()->GetTimeSeconds();

		TargetMesh->SetVisibility(bShowTarget);

		CurrentInputVector = FVector::ZeroVector;
	}
}

void APlayerTargetActor::DisableTargetMovement()
{
	bCurrentlyMovable = false;
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

FVector APlayerTargetActor::GetOwnerControlRotationVector()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	AController* OwnerController = (OwnerPawn) ? OwnerPawn->GetController() : nullptr;
	FVector OwnerControlRotationVector = (OwnerController) ? OwnerController->GetControlRotation().Vector() : FVector::ZeroVector;

	return OwnerControlRotationVector;
}
