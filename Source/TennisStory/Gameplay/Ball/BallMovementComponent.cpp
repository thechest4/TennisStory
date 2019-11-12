// Fill out your copyright notice in the Description page of Project Settings.


#include "BallMovementComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Curves/CurveFloat.h"

#include "DrawDebugHelpers.h"

PRAGMA_DISABLE_OPTIMIZATION

UBallMovementComponent::UBallMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBallMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	GetOwner()->OnActorHit.AddDynamic(this, &UBallMovementComponent::HandleActorHit);
}

void UBallMovementComponent::HandleActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("HandleActorHit"));

	UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
	if (PrimComp)
	{
		PrimComp->SetSimulatePhysics(true);
		bIsFollowingPath = false;
	}
}

void UBallMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsFollowingPath && SplineComp)
	{
		FVector CurrentLocation = GetOwner()->GetActorLocation();
		FVector Direction = SplineComp->FindDirectionClosestToWorldLocation(CurrentLocation, ESplineCoordinateSpace::World);
		FVector NaiveNewLocation = CurrentLocation + Direction * Velocity * DeltaTime;
		FVector SplineNewLocation = SplineComp->FindLocationClosestToWorldLocation(NaiveNewLocation, ESplineCoordinateSpace::World);

		GetOwner()->SetActorLocation(SplineNewLocation, true);
	}
}

void UBallMovementComponent::SetSplineComp(USplineComponent* argSplineComp)
{
	SplineComp = argSplineComp;
}

void UBallMovementComponent::GenerateTrajectory(FVector TargetLocation)
{
	if (!SplineComp)
	{
		return;
	}

	if (!FlatTrajectoryCurve)
	{
		return;
	}

	SplineComp->ClearSplinePoints();

	FVector ActorLoc = GetOwner()->GetActorLocation();
	FVector MidPoint = (TargetLocation - ActorLoc) / 2.f + ActorLoc;
	
	FVector DirectionVec = TargetLocation - ActorLoc;
	DirectionVec.Z = 0.f;
	DirectionVec.Normalize();

	FRichCurve CurveData = FlatTrajectoryCurve->FloatCurve;
	float LeaveSlope = CurveData.GetFirstKey().LeaveTangent;
	float ArriveSlope = CurveData.GetLastKey().ArriveTangent;

	float LeaveAngle = FMath::RadiansToDegrees(FMath::Atan(LeaveSlope));
	float ArriveAngle = FMath::RadiansToDegrees(FMath::Atan(ArriveSlope));

	FVector RightVec = FVector::CrossProduct(FVector::UpVector, DirectionVec);
	FVector StartTangent = DirectionVec.RotateAngleAxis(-LeaveAngle, RightVec);

	FVector EndTangent = DirectionVec.RotateAngleAxis(-ArriveAngle, RightVec);

	SplineComp->AddSplinePoint(ActorLoc, ESplineCoordinateSpace::World, false);
	SplineComp->SetTangentAtSplinePoint(0, StartTangent * 500.f, ESplineCoordinateSpace::Local, false);

	MidPoint += FVector(0.f, 0.f, 150.f);

	SplineComp->AddSplinePoint(MidPoint, ESplineCoordinateSpace::World, false);
	
	SplineComp->AddSplinePoint(TargetLocation, ESplineCoordinateSpace::World, false);
	SplineComp->SetTangentAtSplinePoint(2, EndTangent * 500.f, ESplineCoordinateSpace::Local, false);

	SplineComp->Duration = 10.f;
	SplineComp->UpdateSpline();
}

void UBallMovementComponent::VisualizePath()
{
	if (!SplineComp)
	{
		return;
	}

	if (!SplineMesh)
	{
		return;
	}

	for (float i = 0.f; i < SplineComp->Duration; i += 0.2f)
	{
		FVector SplineLoc = SplineComp->GetLocationAtTime(i, ESplineCoordinateSpace::World);

		DrawDebugSphere(GetWorld(), SplineLoc, 5.0f, 20, FColor::Purple, false, 100.0f);
	}

	bool bCreateSplineMeshComps = false;
	if (!SplineMeshComps.Num())
	{
		bCreateSplineMeshComps = true;
	}

	for (int i = 0; i < SplineComp->GetNumberOfSplinePoints() - 1; i++)
	{
		USplineMeshComponent* SplineMeshComp;
		if (bCreateSplineMeshComps)
		{
			SplineMeshComp = NewObject<USplineMeshComponent>(GetOwner());
			SplineMeshComp->RegisterComponent();
			SplineMeshComp->SetMobility(EComponentMobility::Movable);
			//SplineMeshComp->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false));
			SplineMeshComp->SetStaticMesh(SplineMesh);
		}
		else
		{
			SplineMeshComp = SplineMeshComps[i];
		}

		SplineMeshComp->SetWorldLocationAndRotation(SplineComp->GetOwner()->GetActorLocation(), SplineComp->GetOwner()->GetActorRotation());
		SplineMeshComp->SetStartAndEnd(SplineComp->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local), 
									   SplineComp->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::Local),
									   SplineComp->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::Local), 
									   SplineComp->GetTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::Local));
	}
}

void UBallMovementComponent::StartFollowingPath()
{
	bIsFollowingPath = true;
}
PRAGMA_ENABLE_OPTIMIZATION
