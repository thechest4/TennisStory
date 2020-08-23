// Fill out your copyright notice in the Description page of Project Settings.

#include "TrajectoryTestActor.h"
#include "Gameplay/Ball/BallAimingFunctionLibrary.h"
#include "Components/SplineMeshComponent.h"
#include "Components/SplineComponent.h"

ATrajectoryTestActor::ATrajectoryTestActor()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));

	TrajectorySourceComp = CreateDefaultSubobject<UHighlightableStaticMeshComponent>(TEXT("TrajectorySource"));
	TrajectorySourceComp->SetupAttachment(RootComponent);
	TrajectorySourceComp->SetAbsolute(true, true, true);
	
	TrajectoryEndComp = CreateDefaultSubobject<UHighlightableStaticMeshComponent>(TEXT("TrajectoryEnd"));
	TrajectoryEndComp->SetupAttachment(RootComponent);
	TrajectoryEndComp->SetAbsolute(true, true, true);

	TrajectorySplineComp = CreateDefaultSubobject<USplineComponent>(TEXT("TrajectorySplineComp"));
	TrajectorySplineComp->SetupAttachment(RootComponent);
	TrajectorySplineComp->SetAbsolute(true, true, true);
}

void ATrajectoryTestActor::BeginPlay()
{
	Super::BeginPlay();

	UpdateTrajectory();
	
	SourcePrevPos = TrajectorySourceComp->GetComponentLocation();
	EndPrevPos = TrajectoryEndComp->GetComponentLocation();
}

void ATrajectoryTestActor::Tick(float DeltaSeconds)
{
	if (!HasActorBegunPlay())
	{
		return;
	}

	if (TrajectorySourceComp->GetComponentLocation() != SourcePrevPos || TrajectoryEndComp->GetComponentLocation() != EndPrevPos)
	{
		UpdateTrajectory();

		SourcePrevPos = TrajectorySourceComp->GetComponentLocation();
		EndPrevPos = TrajectoryEndComp->GetComponentLocation();
	}
}

void ATrajectoryTestActor::UpdateTrajectory()
{
	if (TrajectoryCurve)
	{
		FBallTrajectoryData TrajectoryData = UBallAimingFunctionLibrary::GenerateTrajectoryData(TrajectoryCurve, TrajectorySourceComp->GetComponentLocation(), TrajectoryEndComp->GetComponentLocation());
		UBallAimingFunctionLibrary::ApplyTrajectoryDataToSplineComp(TrajectoryData, TrajectorySplineComp);

		UpdateSplineMesh();
	}
}

void ATrajectoryTestActor::UpdateSplineMesh()
{
	if (!SplineMesh)
	{
		return;
	}

	bool bCreateSplineMeshComps = SplineMeshComps.Num() == 0;

	for (int i = 0; i < TrajectorySplineComp->GetNumberOfSplinePoints() - 1; i++)
	{
		USplineMeshComponent* SplineMeshComp;
		if (bCreateSplineMeshComps)
		{
			SplineMeshComp = NewObject<USplineMeshComponent>(this);
			SplineMeshComp->RegisterComponent();
			SplineMeshComp->SetMobility(EComponentMobility::Movable);
			SplineMeshComp->SetStaticMesh(SplineMesh);

			SplineMeshComps.Add(SplineMeshComp);
		}
		else
		{
			SplineMeshComp = SplineMeshComps[i];
		}

		SplineMeshComp->SetStartAndEnd(TrajectorySplineComp->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World), 
									   TrajectorySplineComp->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::World),
									   TrajectorySplineComp->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::World), 
									   TrajectorySplineComp->GetTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::World));
	}
}
