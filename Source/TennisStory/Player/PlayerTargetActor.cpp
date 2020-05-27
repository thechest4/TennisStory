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

			FVector AimVector = GetOwnerControlRotationVector();

			FVector NewTargetLocation = CurrentTargetCourt->GetNextSnapPointLocation(LastSnapPoint, AimVector, static_cast<int>(RightVal), static_cast<int>(ForwardVal));
			SetActorLocation(NewTargetLocation + GetDesiredLocationOffset());
		}
		else
		{
			FVector NewLocation = FVector::ZeroVector;

			if (MovementTarget) //Handle mouse movement/moving towards a movement target
			{
				FVector DirectionVector = MovementTarget->GetActorLocation() - GetActorLocation();
				DirectionVector.Z = 0.f;
				DirectionVector.Normalize();
				
				MovementVector = DirectionVector * MoveSpeed * DeltaSeconds;

				if (MovementVector != FVector::ZeroVector)
				{
					float DistanceToTarget = FVector::Dist2D(MovementTarget->GetActorLocation(), GetActorLocation());
					
					if (DistanceToTarget < MovementVector.Size())
					{
						NewLocation = MovementTarget->GetActorLocation();
					}
					else
					{
						NewLocation = GetActorLocation() + MovementVector;
					}
				}
			}
			else //Normal movement calculation
			{
				MovementVector = MovementVector * MoveSpeed * DeltaSeconds;

				if (MovementVector != FVector::ZeroVector)
				{
					NewLocation = GetActorLocation() + MovementVector;
				}
			}

			if (MovementVector != FVector::ZeroVector)
			{
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
		case ETargetingContext::Volley:
		{
			return ESnapPoint::FrontMid;
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

		FVector SnapPointLocation = CurrentTargetCourt->GetNextSnapPointLocation(LastSnapPoint);

		SetActorLocation(SnapPointLocation + GetDesiredLocationOffset());

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
	MovementTarget = nullptr;
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
