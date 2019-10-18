// Fill out your copyright notice in the Description page of Project Settings.


#include "CamPositioningComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Player/TennisStoryCharacter.h"

#include "DrawDebugHelpers.h"

UCamPositioningComponent::UCamPositioningComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCamPositioningComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerPtr = GetOwner();
	OwnerCamComp = Cast<UCameraComponent>(OwnerPtr->GetComponentByClass(UCameraComponent::StaticClass()));
}

void UCamPositioningComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (OwnerCamComp.IsValid())
	{
		TArray<FVector> LocationsToTrack;

		for (TWeakObjectPtr<AActor> Actor : TrackedActors)
		{
			if (Actor.IsValid())
			{
				if (Actor->IsA<ATennisStoryCharacter>())
				{
					ATennisStoryCharacter* TennisChar = Cast<ATennisStoryCharacter>(Actor);
					FVector CapsuleTop = TennisChar->GetActorLocation() + FVector(0.f, 0.f, TennisChar->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
					FVector CapsuleBottom = TennisChar->GetActorLocation() - FVector(0.f, 0.f, TennisChar->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

					LocationsToTrack.Add(CapsuleTop);
					LocationsToTrack.Add(CapsuleBottom);
				}
				else
				{
					LocationsToTrack.Add(Actor->GetActorLocation());
				}
			}
			else
			{
				TrackedActors.Remove(Actor);
			}
		}

		for (FVector TrackedLocation : LocationsToTrack)
		{
			//DrawDebugSphere(GetWorld(), TrackedLocation, 10.0f, 20, FColor::Blue, true, 0.01f);
		}
	}
}

void UCamPositioningComponent::AddTrackedActor(AActor* ActorToTrack)
{
	TrackedActors.Add(ActorToTrack);
}


