// Fill out your copyright notice in the Description page of Project Settings.

#include "CamPositioningComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Player/TennisStoryCharacter.h"
#include "Gameplay/Ball/TennisBall.h"
#include "Kismet/GameplayStatics.h"
#include "Curves/CurveFloat.h"
#include "Net/UnrealNetwork.h"

UCamPositioningComponent::UCamPositioningComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bReplicates = true;
}

void UCamPositioningComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCamPositioningComponent, TrackedActors);
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

	if (!TrackedActors.Num() || !OwnerPtr.IsValid())
	{
		return;
	}

	UWorld* World = GetWorld();

	checkf(World, TEXT("UCamPositioningComponent::TickComponent - Somehow unable to get a World pointer!"));

	if (OwnerCamComp.IsValid())
	{
		TArray<FVector> LocationsToTrack;

		//Prune the tracked actors list before we do anything
		for (int i = TrackedActors.Num() - 1; i >= 0; i--)
		{
			if (!TrackedActors[i].IsValid() && OwnerPtr->HasAuthority())
			{
				TrackedActors.RemoveAt(i);
			}
		}

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
		}

		APlayerController* LocalPlayerController = GetWorld()->GetGameInstance()->GetFirstLocalPlayerController(GetWorld());
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

			FVector CameraTranslationVector = FVector::ZeroVector;
			float AdjustmentSpeed;

			//Logic to move camera to center of all tracked actors, when other adjustments are not necessary
			{
				FVector2D AverageScreenLocation;

				for (FVector2D ScreenLoc : TrackedLocationsOnScreen)
				{
					AverageScreenLocation += ScreenLoc;
				}

				AverageScreenLocation /= TrackedLocationsOnScreen.Num();

				float ScreenCenterX = ScreenWidth / 2.f;
				float ScreenCenterY = ScreenHeight / 2.f;

				FVector2D ScreenCenterOffset = FVector2D(AverageScreenLocation.X - ScreenCenterX, AverageScreenLocation.Y - ScreenCenterY);
				int LateralOffsetSign = FMath::Sign(ScreenCenterOffset.X);
				int ForwardOffsetSign = FMath::Sign(ScreenCenterOffset.Y);
				
				float LateralOffsetPercent = ScreenCenterOffset.X / ScreenWidth;
				float ForwardOffsetPercent = ScreenCenterOffset.Y / ScreenHeight;

				//Handle Lateral Translation
				{
					FVector CameraRight2D = OwnerPtr->GetActorRightVector().GetSafeNormal2D();
					AdjustmentSpeed = (HorizontalPositioningSpeedCurve) ? HorizontalPositioningSpeedCurve->GetFloatValue(FMath::Abs(LateralOffsetPercent)) : FallbackPositioningSpeed;
					
					if (World->GetTimeSeconds() - LastLateralMoveTimestamp >= MoveDuration || LastLateralMoveTimestamp < 0.f)
					{
						CurrentLateralDirectionSign = LateralOffsetSign;
						LastLateralMoveTimestamp = World->GetTimeSeconds();
					}

					if (LateralOffsetSign == CurrentLateralDirectionSign)
					{
						CameraTranslationVector += CurrentLateralDirectionSign * CameraRight2D * AdjustmentSpeed * DeltaTime;
					}
				}

				//Handle Forward Translation
				{
					FVector CameraForward2D = OwnerPtr->GetActorForwardVector().GetSafeNormal2D();
					AdjustmentSpeed = (HorizontalPositioningSpeedCurve) ? HorizontalPositioningSpeedCurve->GetFloatValue(FMath::Abs(ForwardOffsetPercent)) : FallbackPositioningSpeed;

					if (World->GetTimeSeconds() - LastForwardMoveTimestamp >= MoveDuration || LastForwardMoveTimestamp < 0.f)
					{
						CurrentForwardDirectionSign = ForwardOffsetSign;
						LastForwardMoveTimestamp = World->GetTimeSeconds();
					}

					if (ForwardOffsetSign == CurrentForwardDirectionSign)
					{
						//Subtraction because the screen y-axis is reversed, 0 is the top of the screen
						CameraTranslationVector -= CurrentForwardDirectionSign * CameraForward2D * AdjustmentSpeed * DeltaTime;
					}
				}
			}

			float CalculatedScreenLeft = MarginFromScreenEdges * ScreenWidth;
			float CalculatedScreenRight = (1.f - MarginFromScreenEdges) * ScreenWidth;
			float CalculatedScreenTop = MarginFromScreenEdges * ScreenHeight;
			float CalculatedScreenBottom = (1.f - MarginFromScreenEdges) * ScreenHeight;

			float LeftMargin = 0.f;
			float RightMargin = 0.f;
			float TopMargin = 0.f;
			float BottomMargin = 0.f;

			bool bHasConflictingVerticalAdjustments = false;
			bool bHasConflictingHorizontalAdjustments = false;

			//Cache all margins, normalized such that a negative value indicates off screen and a positive value indicates on screen regardless of which side
			bool bHasInitializedMargins = false;
			for (FVector2D ScreenLocation : TrackedLocationsOnScreen)
			{
				float NewLeft = ScreenLocation.X - CalculatedScreenLeft;
				LeftMargin = (NewLeft < LeftMargin || !bHasInitializedMargins) ? NewLeft : LeftMargin;

				float NewRight = CalculatedScreenRight - ScreenLocation.X;
				RightMargin = (NewRight < RightMargin || !bHasInitializedMargins) ? NewRight : RightMargin;

				float NewTop = ScreenLocation.Y - CalculatedScreenTop;
				TopMargin = (NewTop < TopMargin || !bHasInitializedMargins) ? NewTop : TopMargin;

				float NewBottom = CalculatedScreenBottom - ScreenLocation.Y;
				BottomMargin = (NewBottom < BottomMargin || !bHasInitializedMargins) ? NewBottom : BottomMargin;

				bHasInitializedMargins = true;
			}

			float Margins[] = { LeftMargin / ScreenWidth, RightMargin / ScreenWidth, TopMargin / ScreenHeight, BottomMargin / ScreenHeight };

			int VerticalDirectionSign = 0;
			//If any margin is negative, move the camera up to get a better view
			if (LeftMargin < 0.f || RightMargin < 0.f || TopMargin < 0.f || BottomMargin < 0.f)
			{
				VerticalDirectionSign = 1;
			}
			else
			{
				//If all margins are within our approach margin threshold, lower the camera to get a better view
				float HorzApproachMargin = (ApproachMargin + MarginFromScreenEdges) * ScreenWidth;
				float VertApproachMargin = (ApproachMargin + MarginFromScreenEdges) * ScreenHeight;

				if (LeftMargin >= HorzApproachMargin && RightMargin >= HorzApproachMargin && TopMargin >= VertApproachMargin && BottomMargin >= VertApproachMargin)
				{
					VerticalDirectionSign = -1;
				}
			}

			if (World->GetTimeSeconds() - LastVerticalMoveTimestamp >= MoveDuration || LastVerticalMoveTimestamp < 0.f)
			{
				CurrentVerticalDirectionSign = VerticalDirectionSign;

				if (CurrentVerticalDirectionSign)
				{
					LastVerticalMoveTimestamp = World->GetTimeSeconds();
				}
			}

			FVector CamForward = OwnerPtr->GetActorForwardVector();
			float SmallestMargin = LeftMargin / ScreenWidth;

			for (int i = 0; i < ARRAY_COUNT(Margins); i++)
			{
				if (Margins[i] < SmallestMargin)
				{
					SmallestMargin = Margins[i];
				}
			}

			//Logic for moving camera up
			if (CurrentVerticalDirectionSign > 0)
			{
				AdjustmentSpeed = (PullBackSpeedCurve) ? PullBackSpeedCurve->GetFloatValue(FMath::Abs(SmallestMargin)) : FallbackPositioningSpeed;
				CameraTranslationVector -= CamForward * AdjustmentSpeed * DeltaTime;
			}
			else if (CurrentVerticalDirectionSign < 0) //Logic for moving camera down
			{
				AdjustmentSpeed = (ApproachSpeedCurve) ? ApproachSpeedCurve->GetFloatValue(FMath::Abs(SmallestMargin)) : FallbackPositioningSpeed;
				CameraTranslationVector += CamForward * AdjustmentSpeed * DeltaTime;
			}

			OwnerPtr->SetActorLocation(OwnerPtr->GetActorLocation() + CameraTranslationVector);
		}
	}
}

void UCamPositioningComponent::AddTrackedActor(AActor* ActorToTrack)
{
	if (ActorToTrack)
	{
		TrackedActors.AddUnique(ActorToTrack);
	}
}

void UCamPositioningComponent::StopTrackingActor(AActor* Actor)
{
	if (Actor)
	{
		TrackedActors.Remove(Actor);
	}
}
