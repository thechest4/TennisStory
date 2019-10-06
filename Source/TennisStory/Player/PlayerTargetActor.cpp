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
		MovementVector = MovementVector * MoveSpeed * DeltaSeconds;

		if (MovementVector != FVector::ZeroVector)
		{
			SetActorLocation(GetActorLocation() + MovementVector);
		}
	}
}

void APlayerTargetActor::ShowTargetOnCourt(TWeakObjectPtr<AHalfCourt> CourtToAimAt)
{
	if (CourtToAimAt.IsValid())
	{
		SetActorLocation(CourtToAimAt->GetSnapPointLocation(ESnapPoint::Mid));

		bCurrentlyVisible = true;
		TargetingStartedTime = GetWorld()->GetTimeSeconds();

		TargetMesh->SetVisibility(true);
	}
}

void APlayerTargetActor::HideTarget()
{
	bCurrentlyVisible = false;

	TargetMesh->SetVisibility(false);
}

FVector APlayerTargetActor::ConsumeCurrentInputVector()
{
	LastInputVector = CurrentInputVector;
	CurrentInputVector = FVector::ZeroVector;

	return LastInputVector;
}

