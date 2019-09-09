// Fill out your copyright notice in the Description page of Project Settings.

#include "BallStrikingComponent.h"
#include "Player/TennisStoryCharacter.h"
#include "Player/PlayerTargetActor.h"
#include "Gameplay/TennisRacquet.h"
#include "Gameplay/TennisBall.h"

#include "Engine.h"

UBallStrikingComponent::UBallStrikingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UBallStrikingComponent::AllowBallStriking()
{
	ATennisStoryCharacter* OwnerCharacter = Cast<ATennisStoryCharacter>(GetOwner());
	ATennisRacquet* OwnerRacquet = (OwnerCharacter) ? OwnerCharacter->RacquetActor : nullptr;
	if (OwnerRacquet)
	{
		OwnerRacquet->OverlapDetectionComp->OnComponentBeginOverlap.AddDynamic(this, &UBallStrikingComponent::HandleRacquetOverlapBegin);
	}
}

void UBallStrikingComponent::StopBallStriking()
{
	ATennisStoryCharacter* OwnerCharacter = Cast<ATennisStoryCharacter>(GetOwner());
	ATennisRacquet* OwnerRacquet = (OwnerCharacter) ? OwnerCharacter->RacquetActor : nullptr;
	if (OwnerRacquet)
	{
		OwnerRacquet->OverlapDetectionComp->OnComponentBeginOverlap.RemoveDynamic(this, &UBallStrikingComponent::HandleRacquetOverlapBegin);
	}
}

void UBallStrikingComponent::HandleRacquetOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ATennisStoryCharacter* OwnerCharacter = Cast<ATennisStoryCharacter>(GetOwner());
	APlayerTargetActor* PlayerTarget = (OwnerCharacter) ? OwnerCharacter->TargetActor : nullptr;

	ATennisBall* TennisBall = Cast<ATennisBall>(OtherActor);
	if (PlayerTarget && TennisBall)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Hit a tennis ball!"));

		FVector Trajectory = PlayerTarget->GetActorLocation() - TennisBall->GetActorLocation();
		TennisBall->SetActorRotation(Trajectory.ToOrientationQuat());
		TennisBall->ProjMovementComp->SetVelocityInLocalSpace(FVector(1000.0f, 0.0f, 0.0f));
	}
}
