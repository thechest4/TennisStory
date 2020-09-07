// Fill out your copyright notice in the Description page of Project Settings.

#include "TrajectoryTestActor.h"
#include "Gameplay/Ball/BallAimingFunctionLibrary.h"
#include "Components/SplineMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Components/WidgetComponent.h"

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
		FBallTrajectoryData TrajectoryData;
		
		switch (TrajAlgorithm)
		{
			default:
			case ETrajectoryAlgorithm::Old:
			{
				TrajectoryData = UBallAimingFunctionLibrary::GenerateTrajectoryData(TrajParams_Old, TrajectorySourceComp->GetComponentLocation(), TrajectoryEndComp->GetComponentLocation());
				break;
			}
			case ETrajectoryAlgorithm::New:
			{
				TrajectoryData = UBallAimingFunctionLibrary::GenerateTrajectoryData(TrajParams_New, TrajectorySourceComp->GetComponentLocation(), TrajectoryEndComp->GetComponentLocation());
				break;
			}
		}


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

	for (int i = 0; i < SplineMeshComps_Old.Num(); i++)
	{
		SplineMeshComps_Old[i]->SetHiddenInGame(true);
	}
	
	for (int i = 0; i < SplineMeshComps_New.Num(); i++)
	{
		SplineMeshComps_New[i]->SetHiddenInGame(true);
	}

	TArray<USplineMeshComponent*>* SplineMeshComps = nullptr;
	int NumSegments = 0;

	switch (TrajAlgorithm)
	{
		case ETrajectoryAlgorithm::Old:
		{
			SplineMeshComps = &SplineMeshComps_Old;
			NumSegments = TrajectorySplineComp->GetNumberOfSplinePoints();
			break;
		}
		case ETrajectoryAlgorithm::New:
		{
			SplineMeshComps = &SplineMeshComps_New;
			NumSegments = TrajectorySplineComp->GetNumberOfSplinePoints();
			break;
		}
	}

	bool bCreateSplineMeshComps = (SplineMeshComps) ? SplineMeshComps->Num() == 0 : false;

	for (int i = 0; i < NumSegments; i++)
	{
		USplineMeshComponent* SplineMeshComp;
		if (bCreateSplineMeshComps)
		{
			SplineMeshComp = NewObject<USplineMeshComponent>(this);
			SplineMeshComp->RegisterComponent();
			SplineMeshComp->SetMobility(EComponentMobility::Movable);
			SplineMeshComp->SetStaticMesh(SplineMesh);

			SplineMeshComps->Add(SplineMeshComp);
		}
		else
		{
			SplineMeshComp = (*SplineMeshComps)[i];
			SplineMeshComp->SetHiddenInGame(false);
		}

		switch (TrajAlgorithm)
		{
			case ETrajectoryAlgorithm::Old:
			{
				SplineMeshComp->SetStartAndEnd(TrajectorySplineComp->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World), 
									   TrajectorySplineComp->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::World),
									   TrajectorySplineComp->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::World), 
									   TrajectorySplineComp->GetTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::World));
				break;
			}
			case ETrajectoryAlgorithm::New:
			{
				float StartTime = static_cast<float>(i) / NumSegments;
				float EndTime = static_cast<float>(i + 1) / NumSegments;
		
				SplineMeshComp->SetStartAndEnd(TrajectorySplineComp->GetLocationAtTime(StartTime, ESplineCoordinateSpace::World), 
											   TrajectorySplineComp->GetTangentAtTime(StartTime, ESplineCoordinateSpace::World),
											   TrajectorySplineComp->GetLocationAtTime(EndTime, ESplineCoordinateSpace::World), 
											   TrajectorySplineComp->GetTangentAtTime(EndTime, ESplineCoordinateSpace::World));

				break;
			}
		}
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

void ATrajectoryTestActor::SetCurrentTrajAlgorithm(ETrajectoryAlgorithm NewAlgo)
{
	TrajAlgorithm = NewAlgo;
	UpdateTrajectory();
}

void ATrajectoryTestActor::SetTrajParamsOld(FTrajectoryParams_Old TrajParams)
{
	TrajParams_Old = TrajParams;
	UpdateTrajectory();
}

void ATrajectoryTestActor::SetTrajParamsNew(FTrajectoryParams_New TrajParams)
{
	TrajParams_New = TrajParams;
	UpdateTrajectory();
}

void ATrajectoryTestActor::SetMeshLocations(FVector SourceLocation, FVector EndLocation)
{
	TrajectorySourceComp->SetWorldLocation(SourceLocation);
	TrajectoryEndComp->SetWorldLocation(FVector(EndLocation.X, EndLocation.Y, 1.f));
}
