// Fill out your copyright notice in the Description page of Project Settings.


#include "CamPositioningComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Player/TennisStoryCharacter.h"
#include "Kismet/GameplayStatics.h"

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

		//TODO(achester): Not sure if this is a good way to get the player controller in online play?
		APlayerController* LocalPlayerController = UGameplayStatics::GetPlayerController(this, 0);
		if (LocalPlayerController)
		{
			TArray<FVector2D> TrackedLocationsOnScreen;
			for (FVector TrackedLocation : LocationsToTrack)
			{
				FVector2D LocationOnScreen;
				UGameplayStatics::ProjectWorldToScreen(LocalPlayerController, TrackedLocation, LocationOnScreen);

				TrackedLocationsOnScreen.Add(LocationOnScreen);
			}

			int32 ScreenWidth, ScreenHeight;
			LocalPlayerController->GetViewportSize(ScreenWidth, ScreenHeight);

			float CalculatedScreenLeft = MarginFromScreenEdges * ScreenWidth;
			float CalculatedScreenRight = (1.f - MarginFromScreenEdges) * ScreenWidth;
			float CalculatedScreenTop = MarginFromScreenEdges * ScreenHeight;
			float CalculatedScreenBottom = (1.f - MarginFromScreenEdges) * ScreenHeight;

			FVector CameraForward2D = OwnerPtr->GetActorForwardVector().GetSafeNormal2D();
			FVector CameraRight2D = OwnerPtr->GetActorRightVector().GetSafeNormal2D();

			FVector CameraTranslationVector = FVector::ZeroVector;
			bool bMoveLeft = false;
			bool bMoveRight = false; 
			bool bMoveForward = false; 
			bool bMoveBackward = false;

			for (FVector2D ScreenLocation : TrackedLocationsOnScreen)
			{
				if (ScreenLocation.X < CalculatedScreenLeft && !bMoveLeft)
				{
					CameraTranslationVector -= CameraRight2D;
					bMoveLeft = true;
				}
				else if (ScreenLocation.X > CalculatedScreenRight && !bMoveRight)
				{
					CameraTranslationVector += CameraRight2D;
					bMoveRight = true;
				}

				if (ScreenLocation.Y < CalculatedScreenTop && !bMoveForward)
				{
					CameraTranslationVector += CameraForward2D;
					bMoveForward = true;
				}
				else if (ScreenLocation.Y > CalculatedScreenBottom && !bMoveBackward)
				{
					CameraTranslationVector -= CameraForward2D;
					bMoveBackward = true;
				}
			}

			CameraTranslationVector.Normalize();
			CameraTranslationVector = CameraTranslationVector * PositioningSpeed * DeltaTime;

			OwnerPtr->SetActorLocation(OwnerPtr->GetActorLocation() + CameraTranslationVector);
		}
	}
}

void UCamPositioningComponent::AddTrackedActor(AActor* ActorToTrack)
{
	TrackedActors.Add(ActorToTrack);
}


