// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerTargetActor.h"
#include "Gameplay/HalfCourt.h"
#include "Components/StaticMeshComponent.h"
#include "TennisStoryGameState.h"

APlayerTargetActor::APlayerTargetActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	SetReplicates(true);
	SetReplicateMovement(true);

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	TargetMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetMesh"));
	TargetMesh->SetupAttachment(RootComponent);
	TargetMesh->SetGenerateOverlapEvents(false);
	TargetMesh->SetCollisionProfileName(TEXT("NoCollision"));

	bCurrentlyVisible = false;

	Server_LastMoveTimestamp = 0.f;
	Server_LastConsumedMoveTimestamp = 0.f;
}

void APlayerTargetActor::AddInputVector(FVector Direction, float Value)
{
	CurrentInputVector += Direction * Value;
}

void APlayerTargetActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ATennisStoryCharacter* OwnerChar = Cast<ATennisStoryCharacter>(GetOwner());
	bool bIsLocallyControlled = OwnerChar && OwnerChar->IsLocallyControlled();

	if (bIsLocallyControlled && bCurrentlyVisible && bCurrentlyMovable && CurrentTargetingContext != ETargetingContext::None)
	{
		FVector PrevLocation = GetActorLocation();

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
			SetActorLocation(NewTargetLocation);
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

		//If we are an autonomous proxy, we need to save this move for replication to the server
		if (OwnerChar->GetLocalRole() == ENetRole::ROLE_AutonomousProxy)
		{
			FVector TranslationVector = GetActorLocation() - PrevLocation;

			static const float SIGNIFICANT_MOVE_THRESHOLD = 0.001f;

			//Don't bother saving the move if we didn't go anywhere
			if (TranslationVector.Size() >= SIGNIFICANT_MOVE_THRESHOLD)
			{
				Client_SavedMoves.Add(FTargetSavedMove(TranslationVector, GetWorld()->GetTimeSeconds()));
			}
			
			if (Client_SavedMoves.Num() > 0)
			{
				Server_Move(Client_SavedMoves);
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

		SetActorLocation(SnapPointLocation);

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

bool APlayerTargetActor::Server_Move_Validate(const TArray<FTargetSavedMove>& SavedMoves)
{
	return true;
}

void APlayerTargetActor::Server_Move_Implementation(const TArray<FTargetSavedMove>& SavedMoves)
{
	FVector NetTranslationVector = FVector::ZeroVector;

	for (int i = 0; i < SavedMoves.Num(); i++)
	{
		//Check that the move is more recent than the last move we have consumed, to prevent applying moves multiple times due to network issues
		if (SavedMoves[i].MoveTimestamp > Server_LastConsumedMoveTimestamp)
		{
			NetTranslationVector += SavedMoves[i].TranslationVector;

			//If this is the last SavedMove we received and it's not out of date, record its timestamp - it is the last consumed saved move
			if (i == SavedMoves.Num() - 1)
			{
				Server_LastConsumedMoveTimestamp = SavedMoves[i].MoveTimestamp;
			}
		}
	}
	
	//Figure out the farthest legal move we could have made in the time since our last move, and clamp to that 
	const float MaxPossibleMoveSize = (GetWorld()->GetTimeSeconds() - Server_LastMoveTimestamp) * GetMoveSpeed();
	FVector ClampedTranslationVector = NetTranslationVector.GetClampedToMaxSize(MaxPossibleMoveSize);

	FVector NewLocation = GetActorLocation() + ClampedTranslationVector;

	CurrentTargetCourt->ClampLocationToCourtBounds(NewLocation, GetCourtBoundsContextForTargetingContext(CurrentTargetingContext));
	
	SetActorLocation(NewLocation);

	Client_PruneSavedMoves(Server_LastConsumedMoveTimestamp);
}

void APlayerTargetActor::Client_PruneSavedMoves_Implementation(float LastConsumedMoveTimestamp)
{
	int NumMovesToRemove = 0;
	for (int i = 0; i < Client_SavedMoves.Num(); i++)
	{
		if (Client_SavedMoves[i].MoveTimestamp <= LastConsumedMoveTimestamp)
		{
			NumMovesToRemove++;
		}
	}

	Client_SavedMoves.RemoveAt(0, NumMovesToRemove);
}
