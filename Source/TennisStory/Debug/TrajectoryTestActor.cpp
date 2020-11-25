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
	FBallTrajectoryData TrajectoryData = UBallAimingFunctionLibrary::GenerateTrajectoryData(TrajParams, TrajectorySourceComp->GetComponentLocation(), TrajectoryEndComp->GetComponentLocation(), this);

	UBallAimingFunctionLibrary::ApplyTrajectoryDataToSplineComp(TrajectoryData, TrajectorySplineComp);

	bool bUseValidMat = UBallAimingFunctionLibrary::ValidateTrajectorySplineComp(TrajectoryData, TrajectorySplineComp);

	UpdateSplineMesh(bUseValidMat);
}

void ATrajectoryTestActor::UpdateSplineMesh(bool bUseValidMat)
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
		
		SplineMeshComp->SetStartAndEnd(TrajectorySplineComp->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World), 
										TrajectorySplineComp->GetLeaveTangentAtSplinePoint(i, ESplineCoordinateSpace::World),
										TrajectorySplineComp->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::World),
										TrajectorySplineComp->GetArriveTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::World));

		if (bUseValidMat)
		{
			if (ValidTrajectoryMat)
			{
				SplineMeshComp->SetMaterial(0, ValidTrajectoryMat);
			}
		}
		else
		{
			if (InvalidTrajectoryMat)
			{
				SplineMeshComp->SetMaterial(0, InvalidTrajectoryMat);
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

void ATrajectoryTestActor::UpdateTrajParams(FGameplayTag SourceTag, FGameplayTagContainer ContextTags, FGameplayTag ShotTypeTag, FGameplayTag FallbackTypeTag)
{
	TrajParams = UBallAimingFunctionLibrary::RetrieveTrajectoryParamsFromDataProvider(SourceTag, ContextTags, ShotTypeTag, FallbackTypeTag);
	UpdateTrajectory();
}

void ATrajectoryTestActor::SetMeshLocations(FVector SourceLocation, FVector EndLocation)
{
	TrajectorySourceComp->SetWorldLocation(SourceLocation);
	TrajectoryEndComp->SetWorldLocation(FVector(EndLocation.X, EndLocation.Y, 1.f));
}
