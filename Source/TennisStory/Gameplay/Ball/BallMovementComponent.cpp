// Fill out your copyright notice in the Description page of Project Settings.

#include "BallMovementComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Curves/CurveFloat.h"
#include "Net/UnrealNetwork.h"
#include "TennisStoryGameMode.h"
#include "TennisStoryGameState.h"
#include "Gameplay/Ball/BallAimingFunctionLibrary.h"
#include "Gameplay/Ball/TennisBall.h"
#include "Player/TennisStoryPlayerController.h"
#include "Player/TennisStoryCharacter.h"
#include "../TennisNetActor.h"

UBallMovementComponent::UBallMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);

	CurrentVelocity = 0.f;
	LateralVelocity = 0.f;
	NumBounces = 0;
	CurrentDirection = FVector::ZeroVector;
	CurrentMovementState = EBallMovementState::NotMoving;

	const float TargetFrameDuration = 0.01667f;
	FramesOfBounceLag = 5;
	DurationOfBounceLag = FramesOfBounceLag * TargetFrameDuration;
	CurrentLagTime = 0.f;
	BoundsContextForFirstBounce = EBoundsContext::FullCourt;
	
	TossStartLocation = FVector::ZeroVector;
	TossEndLocation = FVector::ZeroVector;
	CurrentTossAlpha = 0.f;
	TotalTossDuration = 0.f;
}

void UBallMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerPtr = Cast<ATennisBall>(GetOwner());

	if (OwnerPtr->HasAuthority())
	{
		OwnerPtr->OnActorHit.AddDynamic(this, &UBallMovementComponent::HandleActorHit);
	}

	BallCollisionComponent = Cast<UPrimitiveComponent>(OwnerPtr->GetRootComponent());

	TrajectorySplineComp = Cast<USplineComponent>(OwnerPtr->GetComponentByClass(USplineComponent::StaticClass()));
}

void UBallMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBallMovementComponent, CurrentMovementState);
}

void UBallMovementComponent::HandleActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	ATennisStoryGameMode* GameMode = GetWorld()->GetAuthGameMode<ATennisStoryGameMode>();
	if (CurrentMovementState == EBallMovementState::FollowingPath && NumBounces == 0)
	{
		if (OtherActor->IsA<ATennisNetActor>())
		{
			EnterPhysicalMovementState(true, InvertVelocityFromNetHit(0.3f));

			OwnerPtr->OnBallHitNet().Broadcast(BoundsContextForFirstBounce);
			
			if (NetSFX)
			{
				OwnerPtr->Multicast_PlaySound(NetSFX, OwnerPtr->GetActorLocation());
			}
		}
	}
	else if ((CurrentMovementState == EBallMovementState::ContinueUntilHit || CurrentMovementState == EBallMovementState::Physical) 
		&& OwnerPtr->GetCurrentBallState() == ETennisBallState::PlayState)
	{
		//NOTE(achester): this is probably 'ground' specific logic, but since there's nothing else in scene yet the ball can collide with besides net and ground, just handle anything not the net with the below logic
		if (!OtherActor->IsA<ATennisNetActor>())
		{
			if (NumBounces == GameMode->GetAllowedBounces())
			{
				EnterPhysicalMovementState();

				OwnerPtr->OnBallHitBounceLimit().Broadcast();
			}

			const float MinImpulseForParticles = 350.f;
			if (NormalImpulse.Size() > MinImpulseForParticles)
			{
				OwnerPtr->Multicast_SpawnBounceParticleEffect(Hit.ImpactPoint);

				ensureMsgf(OrderedBounceSFX.Num() == 2, TEXT("Not the correct number of sounds in OrderedBounceSFX"));
				if (OrderedBounceSFX.Num() == 2)
				{
					OwnerPtr->Multicast_PlaySound(OrderedBounceSFX[0], OwnerPtr->GetActorLocation());
				}
			}

			NumBounces++;
		}
	}
}

void UBallMovementComponent::DoFirstBounceLogic()
{
	ATennisStoryGameMode* GameMode = GetWorld()->GetAuthGameMode<ATennisStoryGameMode>();
	if (NumBounces < GameMode->GetAllowedBounces())
	{
		ATennisStoryGameState* GameState = GetWorld()->GetGameState<ATennisStoryGameState>();
		TWeakObjectPtr<AHalfCourt> CourtPtr = (GameState && OwnerPtr->LastPlayerToHit.IsValid()) ? GameState->GetCourtToAimAtForCharacter(OwnerPtr->LastPlayerToHit.Get()) : nullptr;
		FVector CurrentLocation = OwnerPtr->GetActorLocation();

		FVector BounceLocation = CurrentLocation;
		BounceLocation.Z = 0.f;

		if (CourtPtr.IsValid() && !CourtPtr->IsLocationInBounds(CurrentLocation, OwnerPtr->GetBallRadius(), BoundsContextForFirstBounce))
		{
			OwnerPtr->OnBallOutOfBounds().Broadcast(BoundsContextForFirstBounce, BounceLocation);
		}

		OwnerPtr->Multicast_SpawnBounceParticleEffect(BounceLocation);

		ensureMsgf(OrderedBounceSFX.Num() == 2, TEXT("Not the correct number of sounds in OrderedBounceSFX"));
		if (OrderedBounceSFX.Num() == 2)
		{
			OwnerPtr->Multicast_PlaySound(OrderedBounceSFX[1], OwnerPtr->GetActorLocation());
		}

		LateralVelocity = CurrentTrajectoryData.ModifiedBounceVelocity;

		NumBounces++;

		//NOTE(achester): Purposely disabling bounce lag after implementing swing forgiveness
		//DoBounceLag();
	}
}

void UBallMovementComponent::OnRep_CurrentMovementState()
{
	if (CurrentMovementState == EBallMovementState::Physical)
	{
		EnterPhysicalMovementState();
	}
	else if (CurrentMovementState == EBallMovementState::NotMoving)
	{
		StopMoving();
	}
}

void UBallMovementComponent::DoBounceLag()
{
	CurrentMovementState = EBallMovementState::BounceLag;
	CurrentLagTime = 0.f;
}

void UBallMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentMovementState == EBallMovementState::BounceLag)
	{
		CurrentLagTime += DeltaTime;

		if (CurrentLagTime >= DurationOfBounceLag)
		{
			CurrentMovementState = EBallMovementState::FollowingPath;
		}
	}
	else if (CurrentMovementState == EBallMovementState::FollowingPath && TrajectorySplineComp)
	{
		FVector CurrentLocation = OwnerPtr->GetActorLocation();
		FVector SplineNewLocation;

		bool bIsBouncing = false;

		//If we haven't bounced yet, detect when we reach the point of the first bounce
		if (NumBounces == 0)
		{
			FVector BounceLocation = TrajectorySplineComp->GetLocationAtSplinePoint(CurrentTrajectoryData.BounceLocationIndex, ESplineCoordinateSpace::World);
			FVector TranslationToBounceLocation = BounceLocation - CurrentLocation;
			float DistanceToBounceLocation = TranslationToBounceLocation.Size();

			//If we're close enough to the bounce location, just snap directly to it
			if (DistanceToBounceLocation <= CurrentVelocity * DeltaTime)
			{
				SplineNewLocation = BounceLocation;
				bIsBouncing = true;

				FVector NextLocation = TrajectorySplineComp->GetLocationAtSplinePoint(CurrentTrajectoryData.BounceLocationIndex + 1, ESplineCoordinateSpace::World);
				CurrentDirection = NextLocation - BounceLocation;
				CurrentDirection.Normalize();
			}
		}

		if (!bIsBouncing)
		{
			//Using trig to find velocity by hypotenuse = adjacentside / cos(theta)
			float CosAngle = FVector::DotProduct(CurrentDirection, LateralDirection);
			CurrentVelocity = (LateralVelocity) / CosAngle;
			FVector NaiveNewLocation = CurrentLocation + CurrentDirection * CurrentVelocity * DeltaTime;

			SplineNewLocation = TrajectorySplineComp->FindLocationClosestToWorldLocation(NaiveNewLocation, ESplineCoordinateSpace::World);

			CurrentDirection = TrajectorySplineComp->FindDirectionClosestToWorldLocation(SplineNewLocation, ESplineCoordinateSpace::World);
		}

		OwnerPtr->SetActorLocation(SplineNewLocation, true);

		//If we're bouncing this frame, do the bounce stuff
		if (bIsBouncing && OwnerPtr->HasAuthority())
		{
			DoFirstBounceLogic();
		}
		
		if (NumBounces > 0 && SplineNewLocation.Equals(CurrentTrajectoryData.TrajectoryEndLocation))
		{
			CurrentMovementState = EBallMovementState::ContinueUntilHit;
		}
	}

	if (CurrentMovementState == EBallMovementState::ContinueUntilHit)
	{
		FVector NewLocation = OwnerPtr->GetActorLocation() + CurrentDirection * CurrentVelocity * DeltaTime;
		OwnerPtr->SetActorLocation(NewLocation, true);
	}

	if (CurrentMovementState == EBallMovementState::ServiceToss)
	{
		FVector NewBallLocation = FVector::ZeroVector;

		if (CurrentTossAlpha <= 1.f) //Toss goes up
		{
			NewBallLocation = FMath::InterpCircularOut(TossStartLocation, TossEndLocation, CurrentTossAlpha);
		}
		else  //Toss goes down
		{
			NewBallLocation = FMath::InterpCircularIn(TossEndLocation, TossStartLocation, CurrentTossAlpha - 1.f);
		}

		OwnerPtr->SetActorLocation(NewBallLocation, true);

		CurrentTossAlpha += DeltaTime / (TotalTossDuration * 0.5f);

		//The toss alpha goes from 0-1 on the way up, then 1-2 on the way down.  An alpha of 2 means we've completed the toss without interruption
		if (CurrentTossAlpha >= 2.f)
		{
			FinishServiceToss(false);
		}
	}
}

void UBallMovementComponent::StartFollowingPath(FBallTrajectoryData TrajectoryData)
{
	CurrentTrajectoryData = TrajectoryData;

	OwnerPtr->SetBallState(ETennisBallState::PlayState);

	UBallAimingFunctionLibrary::ApplyTrajectoryDataToSplineComp(TrajectoryData, TrajectorySplineComp);

	CurrentMovementState = EBallMovementState::FollowingPath;
	CurrentVelocity = TrajectoryData.ModifiedVelocity;
	LateralVelocity = TrajectoryData.ModifiedVelocity;

	CurrentDirection = TrajectorySplineComp->FindDirectionClosestToWorldLocation(OwnerPtr->GetActorLocation(), ESplineCoordinateSpace::World);

	LateralDirection = TrajectoryData.TrajectoryEndLocation - OwnerPtr->GetActorLocation();
	LateralDirection.Z = 0.f;
	LateralDirection.Normalize();

	NumBounces = 0;
	
	if (BallCollisionComponent)
	{
		BallCollisionComponent->SetSimulatePhysics(false);
	}
}

void UBallMovementComponent::StopMoving()
{
	CurrentMovementState = EBallMovementState::NotMoving;
	CurrentVelocity = 0.f;
	LateralVelocity = 0.f;
	CurrentDirection = FVector::ZeroVector;
	
	if (BallCollisionComponent)
	{
		BallCollisionComponent->SetSimulatePhysics(false);
	}
}

void UBallMovementComponent::StartServiceToss(float TossHeight, float argTotalTossDuration)
{
	TossStartLocation = OwnerPtr->GetActorLocation();
	TossEndLocation = OwnerPtr->GetActorLocation() + FVector(0.f ,0.f, TossHeight);
	CurrentTossAlpha = 0.f;
	TotalTossDuration = argTotalTossDuration;
		
	ATennisStoryGameState* TSGameState = GetWorld()->GetGameState<ATennisStoryGameState>();
	ATennisStoryCharacter* ServingChar = (TSGameState) ? TSGameState->GetServingCharacter().Get() : nullptr;
	if (ServingChar)
	{
		ServingChar->DetachBallFromPlayer(OwnerPtr);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("UBallMovementComponent::StartServiceToss - Failed to get ServingChar reference"));
	}

	CurrentMovementState = EBallMovementState::ServiceToss;
}

void UBallMovementComponent::FinishServiceToss(bool bWasInterrupted /*= false*/)
{
	TossStartLocation = FVector::ZeroVector;
	TossEndLocation = FVector::ZeroVector;
	CurrentTossAlpha = 0.f;
	TotalTossDuration = 0.f;

	CurrentMovementState = EBallMovementState::NotMoving;

	ATennisStoryGameState* TSGameState = Cast<ATennisStoryGameState>(GetWorld()->GetGameState());
	ATennisStoryCharacter* CurrentServingCharacter = (TSGameState) ? TSGameState->GetServingCharacter().Get() : nullptr;
	if (!bWasInterrupted && CurrentServingCharacter)
	{
		CurrentServingCharacter->AttachBallToPlayer(OwnerPtr);
	}
	else if (bWasInterrupted && CurrentServingCharacter)
	{
		CurrentServingCharacter->DetachBallFromPlayer(OwnerPtr);
	}
}

void UBallMovementComponent::EnterPhysicalMovementState(bool bOverrideNewVelocity /* = false*/, FVector OverrideNewVelocity /* = FVector::ZeroVector*/)
{
	CurrentMovementState = EBallMovementState::Physical;

	if (BallCollisionComponent && !BallCollisionComponent->IsSimulatingPhysics())
	{
		BallCollisionComponent->SetSimulatePhysics(true);

		FVector NewVelocity = (bOverrideNewVelocity) ? OverrideNewVelocity : CurrentDirection * CurrentVelocity;
		BallCollisionComponent->SetPhysicsLinearVelocity(NewVelocity);
	}
	
	CurrentVelocity = 0.f;
	LateralVelocity = 0.f;
}

FVector UBallMovementComponent::InvertVelocityFromNetHit(float VelocityScale)
{
	const float NewZDirection = -0.2f;
	
	//TODO(achester): This is a very naive way to reflect the ball when hitting the net, reliant on the court being positioned along the X Axis (net across the Y Axis)
	return VelocityScale * CurrentVelocity * FVector(-CurrentDirection.X, CurrentDirection.Y, NewZDirection);
}

#if WITH_EDITORONLY_DATA
void UBallMovementComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UBallMovementComponent, FramesOfBounceLag))
	{
		const float TargetFrameDuration = 0.01667f;

		DurationOfBounceLag = FramesOfBounceLag * TargetFrameDuration;
	}
	
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif
