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

UBallMovementComponent::UBallMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);

	Velocity = 0.f;
	NumBounces = 0;
	CurrentDirection = FVector::ZeroVector;
	CurrentMovementState = EBallMovementState::NotMoving;
	LastPathHeight = 0.f;
	LastPathDistance = 0.f;

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
	if (NumBounces < GameMode->GetAllowedBounces())
	{
		ATennisStoryGameState* GameState = GetWorld()->GetGameState<ATennisStoryGameState>();
		TWeakObjectPtr<AHalfCourt> CourtPtr = (GameState && OwnerPtr->LastPlayerToHit.IsValid()) ? GameState->GetCourtToAimAtForCharacter(OwnerPtr->LastPlayerToHit.Get()) : nullptr;
		FVector CurrentLocation = OwnerPtr->GetActorLocation();

		if (CourtPtr.IsValid() && !CourtPtr->IsLocationInBounds(CurrentLocation, OwnerPtr->GetBallRadius(), BoundsContextForFirstBounce))
		{
			OwnerPtr->OnBallOutOfBounds().Broadcast(BoundsContextForFirstBounce, Hit.ImpactPoint);
		}
		
		GenerateAndFollowBouncePath(Hit);

		OwnerPtr->Multicast_SpawnBounceParticleEffect(Hit.ImpactPoint);

		ensureMsgf(OrderedBounceSFX.Num() == 2, TEXT("Not the correct number of sounds in OrderedBounceSFX"));
		if (OrderedBounceSFX.Num() == 2)
		{
			OwnerPtr->Multicast_PlaySound(OrderedBounceSFX[1], OwnerPtr->GetActorLocation());
		}
	}
	else if (OwnerPtr->GetCurrentBallState() == ETennisBallState::PlayState)
	{
		if (NumBounces == GameMode->GetAllowedBounces())
		{
			EnterPhysicalMovementState();

			OwnerPtr->OnBallHitBounceLimit().Broadcast();
		}

		const float MinImpulseForParticles = 500.f;
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

void UBallMovementComponent::GenerateAndFollowBouncePath(const FHitResult& HitResult)
{
	if (!BounceTrajectoryCurve)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("UBallMovementComponent::GenerateAndFollowBouncePath - BounceTrajectoryCurve was null!"));
		return;
	}

	FVector BallLocation = OwnerPtr->GetActorLocation();

	//TODO(achester): Added a bunch of min values here as a temporary way to prevent weird/bad bounces until I get around to refactoring how bounces work

	float BounceDistance = FMath::Max(LastPathDistance * 0.8f, 800.f);
	FVector BounceEndLocation = HitResult.ImpactPoint + CurrentDirection.GetSafeNormal2D() * BounceDistance;

	float BounceHeight = FMath::Max(LastPathHeight * 0.6f, 100.f); //Min Bounce Height
	BounceHeight = FMath::Min(BounceHeight, 190.f); //Max Bounce Height - Player StrikeZone is 200 cm tall so we need to impose a limit here to prevent bouncing over it
	FBallTrajectoryData TrajectoryData = UBallAimingFunctionLibrary::GenerateTrajectoryData_Old(BounceTrajectoryCurve, BallLocation, BounceEndLocation, BounceHeight, 350.f);

	float BounceVelocity = FMath::Min(Velocity * 0.7f, 1600.f);
	OwnerPtr->Multicast_FollowPath(TrajectoryData, BounceVelocity, false, EBoundsContext::FullCourt, nullptr);
	UBallAimingFunctionLibrary::DebugVisualizeSplineComp(TrajectorySplineComp);
	
	NumBounces++;
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
		FVector Direction = TrajectorySplineComp->FindDirectionClosestToWorldLocation(CurrentLocation, ESplineCoordinateSpace::World);
		FVector NaiveNewLocation = CurrentLocation + Direction * Velocity * DeltaTime;
		FVector SplineNewLocation = TrajectorySplineComp->FindLocationClosestToWorldLocation(NaiveNewLocation, ESplineCoordinateSpace::World);
		
		FVector SplineEndLocation = TrajectorySplineComp->GetWorldLocationAtTime(TrajectorySplineComp->Duration);
		
		if (SplineNewLocation.Equals(SplineEndLocation))
		{
			CurrentMovementState = EBallMovementState::ContinueUntilHit;
		}
		else
		{
			CurrentDirection = SplineNewLocation - CurrentLocation;
			CurrentDirection.Normalize();

			OwnerPtr->SetActorLocation(SplineNewLocation, true);
		}
	}

	if (CurrentMovementState == EBallMovementState::ContinueUntilHit)
	{
		FVector NewLocation = OwnerPtr->GetActorLocation() + CurrentDirection * Velocity * DeltaTime;
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

//NOTE(achester): The spline mesh parts of this function should be moved to another player component related to aiming
//void UBallMovementComponent::VisualizePath()
//{
	//if (!SplineComp)
	//{
	//	return;
	//}

	//if (!SplineMesh)
	//{
	//	return;
	//}

	//for (float i = 0.f; i < SplineComp->Duration; i += 0.2f)
	//{
	//	FVector SplineLoc = SplineComp->GetLocationAtTime(i, ESplineCoordinateSpace::World);

	//	DrawDebugSphere(GetWorld(), SplineLoc, 5.0f, 20, FColor::Purple, false, 100.0f);
	//}

	//bool bCreateSplineMeshComps = false;
	//if (!SplineMeshComps.Num())
	//{
	//	bCreateSplineMeshComps = true;
	//}

	//for (int i = 0; i < SplineComp->GetNumberOfSplinePoints() - 1; i++)
	//{
	//	USplineMeshComponent* SplineMeshComp;
	//	if (bCreateSplineMeshComps)
	//	{
	//		SplineMeshComp = NewObject<USplineMeshComponent>(GetOwner());
	//		SplineMeshComp->RegisterComponent();
	//		SplineMeshComp->SetMobility(EComponentMobility::Movable);
	//		//SplineMeshComp->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false));
	//		SplineMeshComp->SetStaticMesh(SplineMesh);
	//	}
	//	else
	//	{
	//		SplineMeshComp = SplineMeshComps[i];
	//	}

	//	SplineMeshComp->SetWorldLocationAndRotation(SplineComp->GetOwner()->GetActorLocation(), SplineComp->GetOwner()->GetActorRotation());
	//	SplineMeshComp->SetStartAndEnd(SplineComp->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local), 
	//								   SplineComp->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::Local),
	//								   SplineComp->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::Local), 
	//								   SplineComp->GetTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::Local));
	//}
//}

void UBallMovementComponent::StartFollowingPath(FBallTrajectoryData TrajectoryData, float argVelocity, bool bIsFromHit)
{
	OwnerPtr->SetBallState(ETennisBallState::PlayState);

	UBallAimingFunctionLibrary::ApplyTrajectoryDataToSplineComp(TrajectoryData, TrajectorySplineComp);
	LastPathDistance = TrajectoryData.TrajectoryDistance;
	LastPathHeight = TrajectoryData.ApexHeight;

	CurrentMovementState = EBallMovementState::FollowingPath;
	Velocity = argVelocity;

	if (bIsFromHit)
	{	
		NumBounces = 0;
	}
	else
	{
		DoBounceLag();
	}
	
	if (BallCollisionComponent)
	{
		BallCollisionComponent->SetSimulatePhysics(false);
	}
}

void UBallMovementComponent::StopMoving()
{
	CurrentMovementState = EBallMovementState::NotMoving;
	Velocity = 0.f;
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

void UBallMovementComponent::EnterPhysicalMovementState()
{
	CurrentMovementState = EBallMovementState::Physical;

	if (BallCollisionComponent)
	{
		BallCollisionComponent->SetSimulatePhysics(true);
		BallCollisionComponent->SetPhysicsLinearVelocity(CurrentDirection * Velocity);
	}
	
	Velocity = 0.f;
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
