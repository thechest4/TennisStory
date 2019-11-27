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

	bReplicates = true;

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
		GenerateAndFollowBouncePath(Hit);

		OwnerPtr->Multicast_SpawnBounceParticleEffect(Hit.ImpactPoint);
		
		ATennisStoryPlayerController* Controller = (OwnerPtr->LastPlayerToHit.IsValid()) ? Cast<ATennisStoryPlayerController>(OwnerPtr->LastPlayerToHit->Controller) : nullptr;
		ATennisStoryGameState* GameState = GetWorld()->GetGameState<ATennisStoryGameState>();
		TWeakObjectPtr<AHalfCourt> CourtPtr = (GameState) ? GameState->GetCourtToAimAtForPlayer(Controller) : nullptr;
		FVector CurrentLocation = OwnerPtr->GetActorLocation();

		if (CourtPtr.IsValid() && !CourtPtr->IsLocationInBounds(CurrentLocation, OwnerPtr->GetBallRadius()))
		{
			OwnerPtr->OnBallOutOfBounds().Broadcast();
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
	FVector BounceEndLocation = HitResult.ImpactPoint + CurrentDirection.GetSafeNormal2D() * LastPathDistance * 0.6f;
	FBallTrajectoryData TrajectoryData = UBallAimingFunctionLibrary::GenerateTrajectoryData(BounceTrajectoryCurve, BallLocation, BounceEndLocation, LastPathHeight * 0.6f);

	OwnerPtr->Multicast_FollowPath(TrajectoryData, Velocity * 0.7f, false);
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

		CurrentDirection = SplineNewLocation - CurrentLocation;
		CurrentDirection.Normalize();

		OwnerPtr->SetActorLocation(SplineNewLocation, true);
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

void UBallMovementComponent::EnterPhysicalMovementState()
{
	CurrentMovementState = EBallMovementState::Physical;
	Velocity = 0.f;
	
	if (BallCollisionComponent)
	{
		BallCollisionComponent->SetSimulatePhysics(true);
	}
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
