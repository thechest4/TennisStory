// Fill out your copyright notice in the Description page of Project Settings.


#include "TennisBall.h"
#include "TennisStoryGameMode.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"

ATennisBall::ATennisBall()
{
	PrimaryActorTick.bCanEverTick = false;
	
	bReplicates = true;
	
	//NOTE(achester): Currently relying completely on the bReplicateMovement flag for handling ball replication (Movement comp is inactive on clients)
	bReplicateMovement = true;

	CurrentBallState = ETennisBallState::ServiceState;

	RootComponent = BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));

	ProjMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
}

void ATennisBall::BeginPlay()
{
	Super::BeginPlay();

	ApplyBallState();

	if (!HasAuthority())
	{
		ProjMovementComp->SetActive(false);
	}
}

bool ATennisBall::IsInServiceState()
{
	return ProjMovementComp->ProjectileGravityScale == 0.0f;
}

void ATennisBall::SetBallState(ETennisBallState NewState)
{
	if (HasAuthority())
	{
		CurrentBallState = NewState;
		ApplyBallState();
	}
}

void ATennisBall::ApplyBallState()
{
	switch (CurrentBallState)
	{
		case ETennisBallState::PlayState:
		{
			ProjMovementComp->ProjectileGravityScale = 1.f;
			break;
		}
		case ETennisBallState::ServiceState:
		{
			ProjMovementComp->ProjectileGravityScale = 0.f;
			break;
		}
	}
}
