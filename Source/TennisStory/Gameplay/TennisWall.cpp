// Fill out your copyright notice in the Description page of Project Settings.


#include "TennisWall.h"
#include "TennisBall.h"
#include "GameFramework/ProjectileMovementComponent.h"

ATennisWall::ATennisWall()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = WallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
}

void ATennisWall::BeginPlay()
{
	Super::BeginPlay();
	
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("ATennisWall::BeginPlay - Wall was never updated to use BallMovementComponent!"));
	//WallMesh->OnComponentBeginOverlap.AddDynamic(this, &ATennisWall::HandleWallMeshBeginOverlap);
}

void ATennisWall::HandleWallMeshBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	//ATennisBall* TennisBall = Cast<ATennisBall>(OtherActor);
	//if (TennisBall)
	//{
	//	FVector NewTrajectory = TennisBall->GetActorForwardVector() * -1;
	//	TennisBall->SetActorRotation(NewTrajectory.ToOrientationQuat());
	//	//TennisBall->ProjMovementComp->SetVelocityInLocalSpace(FVector(BallReflectionSpeed, 0.0f, 0.0f));
	//}
}

