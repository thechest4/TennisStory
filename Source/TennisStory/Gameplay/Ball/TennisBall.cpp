// Fill out your copyright notice in the Description page of Project Settings.


#include "TennisBall.h"
#include "TennisStoryGameMode.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Gameplay/Ball/BallMovementComponent.h"
#include "Components/SplineComponent.h"
#include "Player/Components/BallStrikingComponent.h"

ATennisBall::ATennisBall()
{
	PrimaryActorTick.bCanEverTick = false;
	
	bReplicates = true;
	
	//NOTE(achester): Currently relying completely on the bReplicateMovement flag for handling ball replication (Movement comp is inactive on clients)
	bReplicateMovement = true;

	CurrentBallState = ETennisBallState::ServiceState;

	RootComponent = BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
	BallMesh->SetCollisionProfileName(TEXT("TennisBall"));

	BallMovementComp = CreateDefaultSubobject<UBallMovementComponent>(TEXT("BallMovementComp"));

	BallTrajectorySplineComp = CreateDefaultSubobject<USplineComponent>(TEXT("BallAimingSplineComp"));
}

void ATennisBall::BeginPlay()
{
	Super::BeginPlay();

	ApplyBallState();
}

bool ATennisBall::IsInServiceState()
{
	return BallMovementComp->GetBallMovementState() == EBallMovementState::NotMoving;
}

void ATennisBall::SetBallState(ETennisBallState NewState)
{
	if (HasAuthority())
	{
		CurrentBallState = NewState;
		ApplyBallState();
	}
}

void ATennisBall::Multicast_FollowPath_Implementation(FBallTrajectoryData TrajectoryData, float Velocity)
{
	//TODO(achester): this is just code copied from UBallStrikingComponent::CopySplineFromData, probably should move that and related code into a static library
	BallTrajectorySplineComp->ClearSplinePoints();

	for (int i = 0; i < TrajectoryData.TrajectoryPoints.Num(); i++)
	{
		BallTrajectorySplineComp->AddSplinePointAtIndex(TrajectoryData.TrajectoryPoints[i].Location, i, ESplineCoordinateSpace::World, false);
		BallTrajectorySplineComp->SetTangentAtSplinePoint(i, TrajectoryData.TrajectoryPoints[i].Tangent, ESplineCoordinateSpace::World, false);
	}

	BallTrajectorySplineComp->UpdateSpline();

	BallMovementComp->StartFollowingPath(Velocity);
}

void ATennisBall::ApplyBallState()
{
	switch (CurrentBallState)
	{
		case ETennisBallState::ServiceState:
		{
			LastPlayerToHit.Reset();
			BallMovementComp->StopMoving();
			break;
		}
	}
}
