// Fill out your copyright notice in the Description page of Project Settings.

#include "BallStrikingComponent.h"
#include "Player/TennisStoryCharacter.h"
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
	ATennisBall* TennisBall = Cast<ATennisBall>(OtherActor);
	if (TennisBall)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Hit a tennis ball!"));
	}
}
