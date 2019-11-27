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
		StartingDistance = FVector::Dist(GetOwner()->GetActorLocation(), ActorToTrack->GetActorLocation());

		if (StartingDistance <= DesiredDistance)
		{
			return;
		}

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
		VisualComp->SetHiddenInGame(true);
}

void UDistanceIndicatorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsVisualizingDistance && ActorToTrack.IsValid())
	{
		float CurrentDistance = FVector::Dist(GetOwner()->GetActorLocation(), ActorToTrack->GetActorLocation());
		float NewAlpha = FMath::Clamp(CurrentDistance / DistanceDifference, 0.f, 1.f);

		//We don't want to switch directions, so if that starts happening just stop the visualization
		if (NewAlpha > CurrentAlpha)
		{
			StopVisualizingDistance();
			return;
		}

		if (NewAlpha <= 0.f)
		{
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
