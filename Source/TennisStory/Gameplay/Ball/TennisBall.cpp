// Fill out your copyright notice in the Description page of Project Settings.


#include "TennisBall.h"
#include "TennisStoryGameMode.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Gameplay/Ball/BallMovementComponent.h"
#include "Components/SplineComponent.h"
#include "Components/DecalComponent.h"
#include "Player/Components/BallStrikingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

ATennisBall::FOnBallSpawnedEvent ATennisBall::BallSpawnedEvent;

ATennisBall::ATennisBall()
{
	PrimaryActorTick.bCanEverTick = false;
	
	bReplicates = true;
	bReplicateMovement = true;

	CurrentBallState = ETennisBallState::ServiceState;

	bWasLastHitAServe = false;
	LastPlayerToHit = nullptr;

	RootComponent = BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
	BallMesh->SetCollisionProfileName(TEXT("TennisBall"));
	BallMesh->SetEnableGravity(true);

	DropShadowDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("Drop Shadow Decal"));
	DropShadowDecal->SetupAttachment(RootComponent);
	DropShadowDecal->SetAbsolute(false, true, true);

	BallMovementComp = CreateDefaultSubobject<UBallMovementComponent>(TEXT("BallMovementComp"));

	BallTrajectorySplineComp = CreateDefaultSubobject<USplineComponent>(TEXT("BallAimingSplineComp"));

	BallTrailParticleEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Ball Trail Component"));
	BallTrailParticleEffect->SetupAttachment(RootComponent);

	bTrailAlwaysOn = false;
}

void ATennisBall::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATennisBall, bWasLastHitAServe);
	DOREPLIFETIME(ATennisBall, LastPlayerToHit);
	DOREPLIFETIME(ATennisBall, CurrentBallState);
}

void ATennisBall::BeginPlay()
{
	Super::BeginPlay();

	ApplyBallState();

	ATennisBall::BallSpawnedEvent.Broadcast(this);

	if (!bTrailAlwaysOn)
	{
		BallTrailParticleEffect->SetActive(false);
	}
}

bool ATennisBall::IsInServiceState()
{
	return BallMovementComp->GetBallMovementState() == EBallMovementState::NotMoving;
}

void ATennisBall::SetBallState(ETennisBallState NewState)
{
	if (HasAuthority())
	{
		CurrentBallState = NewState;
		ApplyBallState();
	}
}

float ATennisBall::GetBallRadius() const
{
	//Hardcoded base radius is half of 1m
	static const float BaseRadius = 50.f;
		
	//Assuming uniform scale
	float ActualBallRadius = BaseRadius * GetActorScale().X;

	return ActualBallRadius;
}

void ATennisBall::StartServiceToss(float TossHeight, float TossDuration)
{
	BallMovementComp->StartServiceToss(TossHeight, TossDuration);
}

void ATennisBall::InterruptServiceToss()
{
	BallMovementComp->FinishServiceToss(true);
}

void ATennisBall::Multicast_InterruptServiceToss_Implementation()
{
	//Handles service hits for the simulated proxy
	if (Role == ROLE_SimulatedProxy)
	{
		InterruptServiceToss();
	}
}

void ATennisBall::Multicast_SpawnBounceLocationParticleEffect_Implementation(FVector Location)
{
	if (BounceLocationParticleEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BounceLocationParticleEffect, Location);
	}
}

void ATennisBall::Multicast_SpawnBounceParticleEffect_Implementation(FVector Location)
{
	if (BounceParticleEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BounceParticleEffect, Location);
	}
}

void ATennisBall::Multicast_SpawnHitParticleEffect_Implementation(UParticleSystem* HitFX, FVector Location)
{
	if (HitFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitFX, Location);
	}
}

void ATennisBall::Multicast_PlaySound_Implementation(USoundBase* Sound, FVector Location)
{
	UGameplayStatics::PlaySoundAtLocation(this, Sound, Location);
}

void ATennisBall::Multicast_FollowPath_Implementation(FBallTrajectoryData TrajectoryData, float Velocity, bool bFromHit, EBoundsContext BoundsContext)
{
	if (!BallTrailParticleEffect->IsActive() && !bTrailAlwaysOn)
	{
		BallTrailParticleEffect->SetActive(true);
	}

	BallMovementComp->StartFollowingPath(TrajectoryData, Velocity, bFromHit);
	BallMovementComp->ProvideBoundsContext(BoundsContext);

	if (HasAuthority() && bFromHit)
	{
		Multicast_SpawnBounceLocationParticleEffect(TrajectoryData.TrajectoryEndLocation);
	}
}

void ATennisBall::OnRep_BallState()
{
	if (CurrentBallState == ETennisBallState::ServiceState)
	{
		if (!bTrailAlwaysOn)
		{
			BallTrailParticleEffect->SetActive(false);
		}
	}
}

void ATennisBall::ApplyBallState()
{
	switch (CurrentBallState)
	{
		case ETennisBallState::ServiceState:
		{
			LastPlayerToHit.Reset();
			BallMovementComp->StopMoving();
			OnRep_BallState();

			break;
		}
	}
}
