// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerTargetActor.h"
#include "Gameplay/HalfCourt.h"
#include "Components/StaticMeshComponent.h"
#include "TennisStoryGameState.h"

APlayerTargetActor::APlayerTargetActor()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = TargetMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetMesh"));
	TargetMesh->SetGenerateOverlapEvents(false);
	TargetMesh->SetCollisionProfileName(TEXT("NoCollision"));

	bCurrentlyVisible = false;
}

void APlayerTargetActor::AddInputVector(FVector Direction, float Value)
{
	CurrentInputVector += Direction * Value;
}

void APlayerTargetActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bCurrentlyVisible && bCurrentlyMovable && CurrentTargetingContext != ETargetingContext::None)
	{
		FVector MovementVector = ConsumeCurrentInputVector();
		MovementVector.Normalize();

		if (CurrentTargetingContext != ETargetingContext::Service && CurrentTargetingMode == ETargetingMode::Simple)
		{
			ESnapPoint SnapPointToAimAt = ESnapPoint::BackMid;

			static const float ValueThreshold = 0.5f; //Any stick value greater than the threshold will be treated as a value of 1, all other values will be treated as values of 0

			float ForwardVal = (FMath::Abs(MovementVector.X) > ValueThreshold) ? 1.f : 0.f;
			ForwardVal *= FMath::Sign(MovementVector.X);
			
			float RightVal = (FMath::Abs(MovementVector.Y) > ValueThreshold) ? 1.f : 0.f;
			RightVal *= FMath::Sign(MovementVector.Y);

			if (ForwardVal == 0.f && RightVal == 0.f)
			{
				//If we have no significant stick input then do nothing
				return;
			}

			if (ForwardVal > 0.f)
			{
				if (RightVal > 0.f)
				{
					SnapPointToAimAt = ESnapPoint::BackRight;
				}
				else if (RightVal < 0.f)
				{
					SnapPointToAimAt = ESnapPoint::BackLeft;
				}
				else
				{
					SnapPointToAimAt = ESnapPoint::BackMid;
				}
			}
			else if (ForwardVal < 0.f)
			{
				if (RightVal > 0.f)
				{
					SnapPointToAimAt = ESnapPoint::FrontRight;
				}
				else if (RightVal < 0.f)
				{
					SnapPointToAimAt = ESnapPoint::FrontLeft;
				}
				else
				{
					SnapPointToAimAt = ESnapPoint::FrontMid;
				}
			}
			else
			{
				if (RightVal > 0.f)
				{
					if (LastSnapPoint == ESnapPoint::BackMid || LastSnapPoint == ESnapPoint::BackLeft || LastSnapPoint == ESnapPoint::BackRight)
					{
						SnapPointToAimAt = ESnapPoint::BackRight;
					}
					else
					{
						SnapPointToAimAt = ESnapPoint::FrontRight;
					}
				}
				else if (RightVal < 0.f)
				{
					if (LastSnapPoint == ESnapPoint::BackMid || LastSnapPoint == ESnapPoint::BackLeft || LastSnapPoint == ESnapPoint::BackRight)
					{
						SnapPointToAimAt = ESnapPoint::BackLeft;
					}
					else
					{
						SnapPointToAimAt = ESnapPoint::FrontLeft;
					}
				}
			}

			FVector AimVector = GetOwnerControlRotationVector();

			LastSnapPoint = SnapPointToAimAt;
			SetActorLocation(CurrentTargetCourt->GetSnapPointLocation(AimVector, SnapPointToAimAt) + GetDesiredLocationOffset());
		}
		else
		{
			MovementVector = MovementVector * MoveSpeed * DeltaSeconds;

			if (MovementVector != FVector::ZeroVector)
			{
				FVector NewLocation = GetActorLocation() + MovementVector;
				CurrentTargetCourt->ClampLocationToCourtBounds(NewLocation, GetCourtBoundsContextForTargetingContext(CurrentTargetingContext));
				SetActorLocation(NewLocation);
			}
		}
	}
}

ESnapPoint APlayerTargetActor::GetStartingSnapPointForTargetingContext(ETargetingContext Context)
{
	switch (Context)
	{
		case ETargetingContext::Service:
		{
			ATennisStoryGameState* TSGameState = GetWorld()->GetGameState<ATennisStoryGameState>();

			checkf(TSGameState, TEXT("APlayerTargetActor::GetStartingSnapPointForTargetingContext - TSGameState was null"))

			EServiceSide ServiceSide = TSGameState->GetServiceSide();

			return (ServiceSide == EServiceSide::Deuce) ? ESnapPoint::ServiceDeuce : ESnapPoint::ServiceAd;
		}
		default:
		case ETargetingContext::GroundStroke:
		{
			return ESnapPoint::BackMid;
		}
	}
}

EBoundsContext APlayerTargetActor::GetCourtBoundsContextForTargetingContext(ETargetingContext TargetingContext)
{
	switch (TargetingContext)
	{
		case ETargetingContext::Service:
		{
			ATennisStoryGameState* TSGameState = GetWorld()->GetGameState<ATennisStoryGameState>();

			checkf(TSGameState, TEXT("APlayerTargetActor::GetStartingSnapPointForTargetingContext - TSGameState was null"))

			EServiceSide ServiceSide = TSGameState->GetServiceSide();

			return (ServiceSide == EServiceSide::Deuce) ? EBoundsContext::ServiceDeuce : EBoundsContext::ServiceAd;
		}
		default:
		{
			return EBoundsContext::FullCourt;
		}
	}
}

void APlayerTargetActor::ShowTargetOnCourt(TWeakObjectPtr<AHalfCourt> CourtToAimAt, bool bShowTarget, ETargetingContext TargetingContext)
{
	if (CourtToAimAt.IsValid())
	{
		CurrentTargetCourt = CourtToAimAt;

		FVector AimVector = GetOwnerControlRotationVector();

		LastSnapPoint = GetStartingSnapPointForTargetingContext(TargetingContext);
		SetActorLocation(CurrentTargetCourt->GetSnapPointLocation(AimVector, LastSnapPoint) + GetDesiredLocationOffset());

		bCurrentlyVisible = true;
		bCurrentlyMovable = true;

		TargetMesh->SetVisibility(bShowTarget);

		CurrentTargetingContext = TargetingContext;
		CurrentInputVector = FVector::ZeroVector;
		CurrentTargetingMode = ETargetingMode::Simple;
	}
}

void APlayerTargetActor::DisableTargetMovement()
{
	bCurrentlyMovable = false;
}

void APlayerTargetActor::HideTarget()
{
	CurrentTargetingContext = ETargetingContext::None;
	bCurrentlyVisible = false;

	TargetMesh->SetVisibility(false);
}

void APlayerTargetActor::BeginPlay()
{
	Super::BeginPlay();

	TargetMesh->SetVisibility(false);
}

FVector APlayerTargetActor::ConsumeCurrentInputVector()
{
	FVector LastInputVector = CurrentInputVector;
	CurrentInputVector = FVector::ZeroVector;

	return LastInputVector;
}

FVector APlayerTargetActor::GetOwnerControlRotationVector()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	AController* OwnerController = (OwnerPawn) ? OwnerPawn->GetController() : nullptr;
	FVector OwnerControlRotationVector = (OwnerController) ? OwnerController->GetControlRotation().Vector() : FVector::ZeroVector;

	return OwnerControlRotationVector;
}
