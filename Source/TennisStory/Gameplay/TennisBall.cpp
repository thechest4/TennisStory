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

