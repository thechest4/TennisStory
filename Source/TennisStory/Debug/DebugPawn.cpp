// Fill out your copyright notice in the Description page of Project Settings.

#include "DebugPawn.h"
#include "Components/HighlightableStaticMeshComponent.h"

#define TraceType_MouseSelect TraceTypeQuery3

ADebugPawn::ADebugPawn()
{
	PrimaryActorTick.bCanEverTick = true;
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

void ADebugPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

