// Fill out your copyright notice in the Description page of Project Settings.


#include "BallMovementComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Curves/CurveFloat.h"
#include "Net/UnrealNetwork.h"

UBallMovementComponent::UBallMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	bReplicates = true;
}

void UBallMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority())
	{
		GetOwner()->OnActorHit.AddDynamic(this, &UBallMovementComponent::HandleActorHit);
	}
	
	BallCollisionComponent = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());

	TrajectorySplineComp = Cast<USplineComponent>(GetOwner()->GetComponentByClass(USplineComponent::StaticClass()));
}

void UBallMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBallMovementComponent, CurrentMovementState);
}

void UBallMovementComponent::HandleActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	EnterPhysicalMovementState();
}

void UBallMovementComponent::OnRep_CurrentMovementState()
{
	if (CurrentMovementState == EBallMovementState::Physical)
	{
		EnterPhysicalMovementState();
	}
	else if (CurrentMovementState == EBallMovementState::NotMoving)
	{
		StopMoving();
	}
}

void UBallMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentMovementState == EBallMovementState::FollowingPath && TrajectorySplineComp)
	{
		FVector CurrentLocation = GetOwner()->GetActorLocation();
		FVector Direction = TrajectorySplineComp->FindDirectionClosestToWorldLocation(CurrentLocation, ESplineCoordinateSpace::World);
		FVector NaiveNewLocation = CurrentLocation + Direction * Velocity * DeltaTime;
		FVector SplineNewLocation = TrajectorySplineComp->FindLocationClosestToWorldLocation(NaiveNewLocation, ESplineCoordinateSpace::World);

		GetOwner()->SetActorLocation(SplineNewLocation, true);
	}
}

//NOTE(achester): The spline mesh parts of this function should be moved to another player component related to aiming
//void UBallMovementComponent::VisualizePath()
//{
	//if (!SplineComp)
	//{
	//	return;
	//}

	//if (!SplineMesh)
	//{
	//	return;
	//}

	//for (float i = 0.f; i < SplineComp->Duration; i += 0.2f)
	//{
	//	FVector SplineLoc = SplineComp->GetLocationAtTime(i, ESplineCoordinateSpace::World);

	//	DrawDebugSphere(GetWorld(), SplineLoc, 5.0f, 20, FColor::Purple, false, 100.0f);
	//}

	//bool bCreateSplineMeshComps = false;
	//if (!SplineMeshComps.Num())
	//{
	//	bCreateSplineMeshComps = true;
	//}

	//for (int i = 0; i < SplineComp->GetNumberOfSplinePoints() - 1; i++)
	//{
	//	USplineMeshComponent* SplineMeshComp;
	//	if (bCreateSplineMeshComps)
	//	{
	//		SplineMeshComp = NewObject<USplineMeshComponent>(GetOwner());
	//		SplineMeshComp->RegisterComponent();
	//		SplineMeshComp->SetMobility(EComponentMobility::Movable);
	//		//SplineMeshComp->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false));
	//		SplineMeshComp->SetStaticMesh(SplineMesh);
	//	}
	//	else
	//	{
	//		SplineMeshComp = SplineMeshComps[i];
	//	}

	//	SplineMeshComp->SetWorldLocationAndRotation(SplineComp->GetOwner()->GetActorLocation(), SplineComp->GetOwner()->GetActorRotation());
	//	SplineMeshComp->SetStartAndEnd(SplineComp->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local), 
	//								   SplineComp->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::Local),
	//								   SplineComp->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::Local), 
	//								   SplineComp->GetTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::Local));
	//}
//}

void UBallMovementComponent::StartFollowingPath(float argVelocity)
{
	CurrentMovementState = EBallMovementState::FollowingPath;
	Velocity = argVelocity;
	
	if (BallCollisionComponent)
	{
		BallCollisionComponent->SetSimulatePhysics(false);
	}
}

void UBallMovementComponent::StopMoving()
{
	CurrentMovementState = EBallMovementState::NotMoving;
	
	if (BallCollisionComponent)
	{
		BallCollisionComponent->SetSimulatePhysics(false);
	}
}

void UBallMovementComponent::EnterPhysicalMovementState()
{
	CurrentMovementState = EBallMovementState::Physical;
	Velocity = 0.f;
	
	if (BallCollisionComponent)
	{
		BallCollisionComponent->SetSimulatePhysics(true);
	}
}
