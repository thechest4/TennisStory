// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerMouseTarget.h"

APlayerMouseTarget::APlayerMouseTarget()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));

	bIsCurrentlyShown = false;
}

void APlayerMouseTarget::BeginPlay()
{
	Super::BeginPlay();
	
	HideTarget();
}

void APlayerMouseTarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsCurrentlyShown())
	{
		FVector2D DirectionVector = ConsumeInputVector();

		FVector TranslationVector = FVector(DirectionVector.X * ForwardMovementSpeed * DeltaTime, DirectionVector.Y * RightMovementSpeed * DeltaTime, 0.f);
		SetActorLocation(GetActorLocation() + TranslationVector);
	}
}

void APlayerMouseTarget::ShowTarget()
{
	bIsCurrentlyShown = true;

	RootComponent->SetVisibility(true, true);
}

void APlayerMouseTarget::HideTarget()
{
	bIsCurrentlyShown = false;

	RootComponent->SetVisibility(false, true);
}

void APlayerMouseTarget::AddForwardInput(float Value)
{
	CurrentInputVector.X += Value;
}

void APlayerMouseTarget::AddRightInput(float Value)
{
	CurrentInputVector.Y += Value;
}

FVector2D APlayerMouseTarget::ConsumeInputVector()
{
	FVector2D ToReturn = CurrentInputVector;

	CurrentInputVector = FVector2D::ZeroVector;

	return ToReturn;
}

