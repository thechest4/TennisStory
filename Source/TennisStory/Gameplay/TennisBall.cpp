// Fill out your copyright notice in the Description page of Project Settings.


#include "TennisBall.h"
#include "TennisStoryGameMode.h"
#include "GameFramework/ProjectileMovementComponent.h"

ATennisBall::ATennisBall()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));

	ProjMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
}

bool ATennisBall::IsInServiceState()
{
	return ProjMovementComp->ProjectileGravityScale == 0.0f;
}

void ATennisBall::SetBallStateForService()
{
	ProjMovementComp->ProjectileGravityScale = 0.f;
}

void ATennisBall::SetBallStateForPlay()
{
	ProjMovementComp->ProjectileGravityScale = 1.f;
}

