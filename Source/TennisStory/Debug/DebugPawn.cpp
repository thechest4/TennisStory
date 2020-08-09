// Fill out your copyright notice in the Description page of Project Settings.

#include "DebugPawn.h"
#include "Components/HighlightableStaticMeshComponent.h"
#include "Debug/TrajectoryTestActor.h"

#define TraceType_MouseSelect TraceTypeQuery3

ADebugPawn::ADebugPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	CurrentHighlightMesh = nullptr;
	CurrentMouseMoveType = EMouseMoveType::None;
	bLockCurrentHighlightMesh = false;
}

void ADebugPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("MouseMoveXY"), IE_Pressed, this, &ADebugPawn::StartXYMove);
	PlayerInputComponent->BindAction(TEXT("MouseMoveXY"), IE_Released, this, &ADebugPawn::StopXYMove);
	
	PlayerInputComponent->BindAction(TEXT("MouseMoveZ"), IE_Pressed, this, &ADebugPawn::StartZMove);
	PlayerInputComponent->BindAction(TEXT("MouseMoveZ"), IE_Released, this, &ADebugPawn::StopZMove);
}

void ADebugPawn::StartXYMove()
{
	if (CurrentMouseMoveType == EMouseMoveType::None && CurrentHighlightMesh)
	{
		CurrentMouseMoveType = EMouseMoveType::XY;
		bLockCurrentHighlightMesh = true;
	}
}

void ADebugPawn::StopXYMove()
{
	if (CurrentMouseMoveType == EMouseMoveType::XY)
	{
		CurrentMouseMoveType = EMouseMoveType::None;
		bLockCurrentHighlightMesh = false;
	}
}

void ADebugPawn::StartZMove()
{
	if (CurrentMouseMoveType == EMouseMoveType::None && CurrentHighlightMesh)
	{
		CurrentMouseMoveType = EMouseMoveType::Z;
		bLockCurrentHighlightMesh = true;
	}
}

void ADebugPawn::StopZMove()
{
	if (CurrentMouseMoveType == EMouseMoveType::Z)
	{
		CurrentMouseMoveType = EMouseMoveType::None;
		bLockCurrentHighlightMesh = false;
	}
}

void ADebugPawn::BeginPlay()
{
	Super::BeginPlay();

	PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		PC->bShowMouseCursor = true;
	}
}

void ADebugPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FHitResult OutHit;
	PC->GetHitResultUnderCursorByChannel(TraceType_MouseSelect, false, OutHit);

	if (!bLockCurrentHighlightMesh)
	{
		UHighlightableStaticMeshComponent* HighlightMesh = Cast<UHighlightableStaticMeshComponent>(OutHit.Component);
		if (HighlightMesh)
		{
			if (CurrentHighlightMesh && HighlightMesh != CurrentHighlightMesh)
			{
				CurrentHighlightMesh->EndHighlight();
			}

			if (HighlightMesh != CurrentHighlightMesh)
			{
				CurrentHighlightMesh = HighlightMesh;
				CurrentHighlightMesh->StartHighlight(HighlightMat);
			}
		}
		else if (CurrentHighlightMesh)
		{
			CurrentHighlightMesh->EndHighlight();
			CurrentHighlightMesh = nullptr;
		}
	}

	float MouseXDelta, MouseYDelta;
	PC->GetInputMouseDelta(MouseXDelta, MouseYDelta);

	if (CurrentMouseMoveType != EMouseMoveType::None && CurrentHighlightMesh)
	{
		ATrajectoryTestActor* TrajectoryTestActor = Cast<ATrajectoryTestActor>(CurrentHighlightMesh->GetOwner());
		if (TrajectoryTestActor)
		{
			TrajectoryTestActor->RequestMoveComponent(CurrentHighlightMesh, CurrentMouseMoveType, GetActorRightVector(), MouseXDelta, MouseYDelta);
		}
	}
}

