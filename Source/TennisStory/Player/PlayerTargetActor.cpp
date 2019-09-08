// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerTargetActor.h"

APlayerTargetActor::APlayerTargetActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APlayerTargetActor::AddInputVector(FVector Direction, float Value)
{
	CurrentInputVector += Direction * Value;
}

void APlayerTargetActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FVector MovementVector = ConsumeCurrentInputVector();
	MovementVector = MovementVector * MoveSpeed * DeltaSeconds;

	if (MovementVector != FVector::ZeroVector)
	{
		SetActorLocation(GetActorLocation() + MovementVector);
	}
}

FVector APlayerTargetActor::ConsumeCurrentInputVector()
{
	LastInputVector = CurrentInputVector;
	CurrentInputVector = FVector::ZeroVector;

	return LastInputVector;
}

