// Fill out your copyright notice in the Description page of Project Settings.

#include "TrajectoryTestActor.h"

ATrajectoryTestActor::ATrajectoryTestActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));

	TrajectorySourceComp = CreateDefaultSubobject<UHighlightableStaticMeshComponent>(TEXT("TrajectorySource"));
	TrajectorySourceComp->SetupAttachment(RootComponent);
	TrajectorySourceComp->SetAbsolute(true, true, true);

	AllowedMoveTypes_TrajectorySource = { EMouseMoveType::XY, EMouseMoveType::Z };
	
	TrajectoryEndComp = CreateDefaultSubobject<UHighlightableStaticMeshComponent>(TEXT("TrajectoryEnd"));
	TrajectoryEndComp->SetupAttachment(RootComponent);
	TrajectoryEndComp->SetAbsolute(true, true, true);

	AllowedMoveTypes_TrajectoryEnd = { EMouseMoveType::XY };
}

void ATrajectoryTestActor::BeginPlay()
{
	Super::BeginPlay();
}

void ATrajectoryTestActor::RequestMoveComponent(UHighlightableStaticMeshComponent* CompToMove, EMouseMoveType MoveType, FVector RightVector, float XDelta, float YDelta)
{
	bool bIsAllowedMoveType = false;

	if (CompToMove == TrajectorySourceComp)
	{
		bIsAllowedMoveType = AllowedMoveTypes_TrajectorySource.Contains(MoveType);
	}
	else if (CompToMove == TrajectoryEndComp)
	{
		bIsAllowedMoveType = AllowedMoveTypes_TrajectoryEnd.Contains(MoveType);
	}
	else
	{
		//CompToMove doesn't belong to us
		return;
	}

	if (!bIsAllowedMoveType)
	{
		return;
	}

	if (MoveType == EMouseMoveType::XY)
	{
		FVector RightTranslation = RightVector * XDelta * MoveSpeed;

		FVector ForwardDir = FVector::CrossProduct(RightVector, FVector::UpVector);
		FVector ForwardTranslation = ForwardDir * YDelta * MoveSpeed;
		
		CompToMove->AddWorldOffset(RightTranslation + ForwardTranslation);
	}
	else if (MoveType == EMouseMoveType::Z)
	{
		FVector UpTranslation = FVector::UpVector * YDelta * MoveSpeed;

		CompToMove->AddWorldOffset(UpTranslation);
	}
}
