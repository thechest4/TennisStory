// Fill out your copyright notice in the Description page of Project Settings.

#include "BallMovementComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Curves/CurveFloat.h"
#include "Net/UnrealNetwork.h"
#include "TennisStoryGameMode.h"
#include "Gameplay/Ball/BallAimingFunctionLibrary.h"
#include "Gameplay/Ball/TennisBall.h"

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
	DurationOfBounceLag = 5 * 0.01667f; //Default lag value is 5 frames, at 60 fps
	CurrentLagTime = 0.f;
}

void UBallMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority())
	{
		GetOwner()->OnActorHit.AddDynamic(this, &UBallMovementComponent::HandleActorHit);
	}
	
	BallCollisionComponent = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());

	TrajectorySplineComp = Cast<USplineComponent>(GetOwner()->GetComponentByClass(USplineComponent::StaticClass()));
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

		ATennisBall* TennisBall = Cast<ATennisBall>(SelfActor);
		if (TennisBall)
		{
			TennisBall->Multicast_SpawnBounceParticleEffect(Hit.ImpactPoint);
		}
	}
	else
	{
		EnterPhysicalMovementState();

		ATennisBall* TennisBall = Cast<ATennisBall>(SelfActor);
		if (TennisBall)
		{
			TennisBall->Multicast_SpawnBounceParticleEffect(Hit.ImpactPoint);
		}
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

	FVector BallLocation = GetOwner()->GetActorLocation();
	FVector BounceEndLocation = HitResult.ImpactPoint + CurrentDirection.GetSafeNormal2D() * LastPathDistance * 0.6f;
	FBallTrajectoryData TrajectoryData = UBallAimingFunctionLibrary::GenerateTrajectoryData(BounceTrajectoryCurve, BallLocation, BounceEndLocation, LastPathHeight * 0.6f);

	ATennisBall* Owner = Cast<ATennisBall>(GetOwner());
	if (Owner)
	{
		Owner->Multicast_FollowPath(TrajectoryData, Velocity * 0.7f, false);
		UBallAimingFunctionLibrary::DebugVisualizeSplineComp(TrajectorySplineComp);
	}

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
		FVector CurrentLocation = GetOwner()->GetActorLocation();
		FVector Direction = TrajectorySplineComp->FindDirectionClosestToWorldLocation(CurrentLocation, ESplineCoordinateSpace::World);
		FVector NaiveNewLocation = CurrentLocation + Direction * Velocity * DeltaTime;
		FVector SplineNewLocation = TrajectorySplineComp->FindLocationClosestToWorldLocation(NaiveNewLocation, ESplineCoordinateSpace::World);

		CurrentDirection = SplineNewLocation - CurrentLocation;
		CurrentDirection.Normalize();

		GetOwner()->SetActorLocation(SplineNewLocation, true);
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
