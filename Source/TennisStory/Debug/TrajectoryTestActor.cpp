// Fill out your copyright notice in the Description page of Project Settings.

#include "TrajectoryTestActor.h"
#include "Gameplay/Ball/BallAimingFunctionLibrary.h"
#include "Components/SplineMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Components/WidgetComponent.h"
#include "../Components/HighlightableStaticMeshComponent.h"
#include "TrajActorContextMenu.h"

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

	ContextMenuComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("Context Menu Component"));
	ContextMenuComp->SetupAttachment(TrajectorySourceComp);
	ContextMenuComp->SetWidgetSpace(EWidgetSpace::Screen);
}

void ATrajectoryTestActor::BeginPlay()
{
	Super::BeginPlay();

	UpdateTrajectory();
	
	SourcePrevPos = TrajectorySourceComp->GetComponentLocation();
	EndPrevPos = TrajectoryEndComp->GetComponentLocation();

	UTrajActorContextMenu* ContextMenu = Cast<UTrajActorContextMenu>(ContextMenuComp->GetUserWidgetObject());
	if (ContextMenu)
	{
		ContextMenu->SetVisibility(ESlateVisibility::Hidden);
		ContextMenu->SetTrajActorRef(this);
	}

	TArray<ECursorMoveType> SourceMoveTypes = { ECursorMoveType::XY, ECursorMoveType::Z };
	TrajectorySourceComp->SetAllowedMoveTypes(SourceMoveTypes);
	
	TArray<ECursorMoveType> EndMoveTypes = { ECursorMoveType::XY};
	TrajectoryEndComp->SetAllowedMoveTypes(EndMoveTypes);
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
		FBallTrajectoryData TrajectoryData = UBallAimingFunctionLibrary::GenerateTrajectoryData(TrajParams, TrajectorySourceComp->GetComponentLocation(), TrajectoryEndComp->GetComponentLocation(), this);

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

	int NumSegments = TrajectorySplineComp->GetNumberOfSplinePoints();

	bool bCreateSplineMeshComps = SplineMeshComps.Num() == 0;

	for (int i = 0; i < NumSegments; i++)
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
			SplineMeshComp->SetHiddenInGame(false);
		}

		
		float StartTime = static_cast<float>(i) / NumSegments;
		float EndTime = static_cast<float>(i + 1) / NumSegments;
		
		SplineMeshComp->SetStartAndEnd(TrajectorySplineComp->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World), 
										TrajectorySplineComp->GetLeaveTangentAtSplinePoint(i, ESplineCoordinateSpace::World),
										TrajectorySplineComp->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::World),
										TrajectorySplineComp->GetArriveTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::World));
	}
}

void ATrajectoryTestActor::ShowContextMenu()
{
	UUserWidget* UserWidget = ContextMenuComp->GetUserWidgetObject();
	if (UserWidget)
	{
		UserWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void ATrajectoryTestActor::HideContextMenu()
{
	UUserWidget* UserWidget = ContextMenuComp->GetUserWidgetObject();
	if (UserWidget)
	{
		UserWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ATrajectoryTestActor::SetTrajParams(FTrajectoryParams argTrajParams)
{
	TrajParams = argTrajParams;
	UpdateTrajectory();
}

void ATrajectoryTestActor::SetMeshLocations(FVector SourceLocation, FVector EndLocation)
{
	TrajectorySourceComp->SetWorldLocation(SourceLocation);
	TrajectoryEndComp->SetWorldLocation(FVector(EndLocation.X, EndLocation.Y, 1.f));
}
