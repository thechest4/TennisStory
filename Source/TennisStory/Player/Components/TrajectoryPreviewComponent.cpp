// Fill out your copyright notice in the Description page of Project Settings.

#include "TrajectoryPreviewComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/SplineComponent.h"
#include "BallStrikingComponent.h"

UTrajectoryPreviewComponent::UTrajectoryPreviewComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	bIsCurrentlyShowingTrajectory = false;
	PrevStartLocation = FVector::ZeroVector;
	PrevEndLocation = FVector::ZeroVector;
}

void UTrajectoryPreviewComponent::SetBallStrikingCompReference(UBallStrikingComponent* BallStrikingComp)
{
	//Currently only supporting binding once and not rebinding
	if (OwnerBallStrikingComp.IsValid())
	{
		return;
	}
	
	OwnerBallStrikingComp = BallStrikingComp;

	if (OwnerBallStrikingComp.IsValid())
	{
		OwnerBallStrikingComp->OnShotTagsChanged().AddUObject(this, &UTrajectoryPreviewComponent::HandleShotTagsChanged);		
	}
}

void UTrajectoryPreviewComponent::HandleShotTagsChanged()
{
	ensureMsgf(OwnerBallStrikingComp.IsValid(), TEXT("OwnerBallStrikingComp was null"));

	FGameplayTag ShotSource = OwnerBallStrikingComp->CurrentShotSourceTag;
	FGameplayTagContainer ShotContext = OwnerBallStrikingComp->CurrentShotContextTags;
	FGameplayTag DesiredShotType = OwnerBallStrikingComp->DesiredShotTypeTag;
	FGameplayTag FallbackShotType = OwnerBallStrikingComp->CurrentFallbackShotTypeTag;

	CurrentTrajParams = UBallAimingFunctionLibrary::RetrieveTrajectoryParamsFromDataProvider(ShotSource, ShotContext, DesiredShotType, FallbackShotType);
}

void UTrajectoryPreviewComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsCurrentlyShowingTrajectory && StartLocObjPtr.IsValid() && EndLocObjPtr.IsValid() && OwnerSplinePreviewComp.IsValid())
	{
		if (GetObjectLocation(StartLocObjPtr.Get()) != PrevStartLocation || GetObjectLocation(EndLocObjPtr.Get()) != PrevEndLocation)
		{
			GeneratePreviewTrajectory();
		}
	}
}

void UTrajectoryPreviewComponent::GeneratePreviewTrajectory()
{
	FBallTrajectoryData TrajectoryData = UBallAimingFunctionLibrary::GenerateTrajectoryData(CurrentTrajParams, GetObjectLocation(StartLocObjPtr.Get()), GetObjectLocation(EndLocObjPtr.Get()));

	UBallAimingFunctionLibrary::ApplyTrajectoryDataToSplineComp(TrajectoryData, OwnerSplinePreviewComp.Get());

	bool bUseValidMat = UBallAimingFunctionLibrary::ValidateTrajectorySplineComp(TrajectoryData, OwnerSplinePreviewComp.Get());

	if (!SplineMesh)
	{
		return;
	}

	int NumSegments = TrajectoryData.BounceLocationIndex;

	bool bCreateSplineMeshComps = SplineMeshComps.Num() == 0;

	for (int i = 0; i < NumSegments; i++)
	{
		USplineMeshComponent* SplineMeshComp;
		if (bCreateSplineMeshComps)
		{
			SplineMeshComp = NewObject<USplineMeshComponent>(GetOwner());
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

		SplineMeshComp->SetStartAndEnd(OwnerSplinePreviewComp->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World),
			OwnerSplinePreviewComp->GetLeaveTangentAtSplinePoint(i, ESplineCoordinateSpace::World),
			OwnerSplinePreviewComp->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::World),
			OwnerSplinePreviewComp->GetArriveTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::World));

		if (bUseValidMat)
		{
			if (ValidSplineMeshMat)
			{
				SplineMeshComp->SetMaterial(0, ValidSplineMeshMat);
			}
		}
		else
		{
			if (InvalidSplineMeshMat)
			{
				SplineMeshComp->SetMaterial(0, InvalidSplineMeshMat);
			}
		}
	}
}

FVector UTrajectoryPreviewComponent::GetObjectLocation(UObject* Object)
{
	AActor* ObjToActor = Cast<AActor>(Object);
	if (ObjToActor)
	{
		return ObjToActor->GetActorLocation();
	}

	USceneComponent* ObjToSceneComp = Cast<USceneComponent>(Object);
	if (ObjToSceneComp)
	{
		return ObjToSceneComp->GetComponentLocation();
	}

	checkNoEntry()

	return FVector::ZeroVector;
}

void UTrajectoryPreviewComponent::StartShowingTrajectory(USplineComponent* SplinePreviewComp, UObject* StartLocObj, UObject* EndLocObj)
{
	if (!StartLocObj || !EndLocObj)
	{
		return;
	}

	bIsCurrentlyShowingTrajectory = true;

	OwnerSplinePreviewComp = SplinePreviewComp;
	StartLocObjPtr = StartLocObj;
	EndLocObjPtr = EndLocObj;

	PrevStartLocation = GetObjectLocation(StartLocObjPtr.Get());
	PrevEndLocation = GetObjectLocation(EndLocObjPtr.Get());

	CurrentTrajParams = UBallAimingFunctionLibrary::RetrieveTrajectoryParamsFromDataProvider(OwnerBallStrikingComp->CurrentShotSourceTag, OwnerBallStrikingComp->CurrentShotContextTags, OwnerBallStrikingComp->DesiredShotTypeTag, OwnerBallStrikingComp->CurrentFallbackShotTypeTag);

	GeneratePreviewTrajectory();
}

void UTrajectoryPreviewComponent::StopShowingTrajectory()
{
	bIsCurrentlyShowingTrajectory = false;

	OwnerSplinePreviewComp = nullptr;
	StartLocObjPtr = nullptr;
	EndLocObjPtr = nullptr;

	for (int i = 0; i < SplineMeshComps.Num(); i++)
	{
		SplineMeshComps[i]->SetHiddenInGame(true);
	}
}
