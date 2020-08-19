// Fill out your copyright notice in the Description page of Project Settings.

#include "TrajectoryTestActor.h"

ATrajectoryTestActor::ATrajectoryTestActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));

	TrajectorySourceComp = CreateDefaultSubobject<UHighlightableStaticMeshComponent>(TEXT("TrajectorySource"));
	TrajectorySourceComp->SetupAttachment(RootComponent);
	TrajectorySourceComp->SetAbsolute(true, true, true);
	
	TrajectoryEndComp = CreateDefaultSubobject<UHighlightableStaticMeshComponent>(TEXT("TrajectoryEnd"));
	TrajectoryEndComp->SetupAttachment(RootComponent);
	TrajectoryEndComp->SetAbsolute(true, true, true);
}

void ATrajectoryTestActor::BeginPlay()
{
	Super::BeginPlay();
}
