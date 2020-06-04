// Fill out your copyright notice in the Description page of Project Settings.

#include "DistanceIndicatorComponent.h"

UDistanceIndicatorComponent::UDistanceIndicatorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	MinDistanceForVisualization = 300.f;
	DesiredDistance = 500.f;
	MaxScale = 2.f;
	MinScale = 0.1f;
	
	bIsVisualizingDistance = false;
	CurrentAlpha = 0.f;
	StartingDistance = 0.f;
}

void UDistanceIndicatorComponent::StartVisualizingDistance(TWeakObjectPtr<AActor> argActorToTrack)
{
	if (argActorToTrack.IsValid() && VisualComp)
	{
		ActorToTrack = argActorToTrack;

		//Desired distance is the distance at which the alpha will be 0, MinDistanceForVisualization is the minimum distance we need to start with for visualization to activate
		StartingDistance = FVector::Dist(GetOwner()->GetActorLocation(), ActorToTrack->GetActorLocation());

		if (StartingDistance <= DesiredDistance)
		{
			return;
		}

		//DistanceDifference is the initial distance from our location, compensating for the desired distance (We don't want to get TO the tracked actor location, we want to get desireddistance cm away)
		DistanceDifference = StartingDistance - DesiredDistance;

		if (DistanceDifference < MinDistanceForVisualization)
		{
			return;
		}

		bIsVisualizingDistance = true;
		VisualComp->SetHiddenInGame(false);
		
		CurrentAlpha = FMath::Clamp(StartingDistance / DistanceDifference, 0.f, 1.f);
	}
}

void UDistanceIndicatorComponent::StopVisualizingDistance()
{
	ActorToTrack.Reset();
	bIsVisualizingDistance = false;

	if (VisualComp)
	{
		VisualComp->SetHiddenInGame(true);
	}
}

void UDistanceIndicatorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsVisualizingDistance && ActorToTrack.IsValid())
	{
		float CurrentDistance = FVector::Dist(GetOwner()->GetActorLocation(), ActorToTrack->GetActorLocation());
		float NewAlpha = FMath::Clamp((CurrentDistance - DesiredDistance) / DistanceDifference, 0.f, 1.f);

		if (NewAlpha <= 0.f)
		{
			TargetReachedEvent.Broadcast();
			StopVisualizingDistance();
			return;
		}

		CurrentAlpha = NewAlpha;

		float NewScale = FMath::Lerp(MinScale, MaxScale, CurrentAlpha);

		VisualComp->SetWorldScale3D(FVector(NewScale));
	}
	else if (bIsVisualizingDistance && !ActorToTrack.IsValid())
	{
		StopVisualizingDistance();
	}
}
