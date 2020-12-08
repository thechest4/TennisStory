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
#include "Player/Components/DistanceIndicatorComponent.h"
#include "../TrajectoryDataProvider.h"
#include <Components/SphereComponent.h>

ATennisBall::FOnBallSpawnedEvent ATennisBall::BallSpawnedEvent;

ATennisBall::ATennisBall()
{
	PrimaryActorTick.bCanEverTick = false;
	
	SetReplicates(true);
	SetReplicateMovement(true);

	CurrentBallState = ETennisBallState::ServiceState;

	bWasLastHitAServe = false;
	LastPlayerToHit = nullptr;

	RootComponent = BallCollisionBody = CreateDefaultSubobject<USphereComponent>(TEXT("BallCollisionBody"));
	BallCollisionBody->SetCollisionProfileName(TEXT("TennisBall"));
	BallCollisionBody->SetEnableGravity(true);
	BallCollisionBody->SetSphereRadius(DefaultBallRadius);

	BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
	BallMesh->SetupAttachment(RootComponent);
	BallMesh->SetAbsolute(false, false, true);
	BallMesh->SetCollisionProfileName(TEXT("NoCollision"));

	DropShadowDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("Drop Shadow Decal"));
	DropShadowDecal->SetupAttachment(RootComponent);
	DropShadowDecal->SetAbsolute(false, true, true);

	BallMovementComp = CreateDefaultSubobject<UBallMovementComponent>(TEXT("BallMovementComp"));

	BallTrajectorySplineComp = CreateDefaultSubobject<USplineComponent>(TEXT("BallAimingSplineComp"));

	BallTrailParticleEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Ball Trail Component"));
	BallTrailParticleEffect->SetupAttachment(RootComponent);

	bTrailAlwaysOn = false;
	
	DistanceIndicatorRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DistanceIndicatorRingMesh"));
	DistanceIndicatorRing->SetupAttachment(RootComponent);
	DistanceIndicatorRing->SetCollisionProfileName(TEXT("NoCollision"));
	DistanceIndicatorRing->SetHiddenInGame(true);
	DistanceIndicatorRing->SetAbsolute(false, true, true);

	DistanceIndicatorComp = CreateDefaultSubobject<UDistanceIndicatorComponent>(TEXT("DistanceIndicatorComp"));
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

	DistanceIndicatorComp->VisualComp = DistanceIndicatorRing;
	DistanceIndicatorComp->OnTargetReached().AddUObject(this, &ATennisBall::HandleDistanceIndicatorTargetReached);
	DynamicBallMat = BallMesh->CreateDynamicMaterialInstance(0);
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
	return BallCollisionBody->GetScaledSphereRadius();
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
	if (GetLocalRole() == ROLE_SimulatedProxy)
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

void ATennisBall::Multicast_FollowPath_Implementation(FBallTrajectoryData TrajectoryData, EBoundsContext BoundsContext, ATennisStoryCharacter* PlayerWhoHitBall)
{
	if (!BallTrailParticleEffect->IsActive() && !bTrailAlwaysOn)
	{
		BallTrailParticleEffect->SetActive(true);
	}

	SetBallTrailColor(TrajectoryData.ShotTypeTag);

	BallMovementComp->StartFollowingPath(TrajectoryData);
	BallMovementComp->ProvideBoundsContext(BoundsContext);

	if (HasAuthority())
	{
		FVector BounceLocation = TrajectoryData.TrajectoryPoints[TrajectoryData.BounceLocationIndex].Location;
		BounceLocation.Z = 0;

		//NOTE(achester): Purposely disabling bounce location indicator after implementing swing forgiveness
		//Multicast_SpawnBounceLocationParticleEffect(BounceLocation);
	}

	ATennisStoryGameState* TSGameState = GetWorld()->GetGameState<ATennisStoryGameState>();
	if (TSGameState && PlayerWhoHitBall)
	{
		int LastPlayerToHitTeamId = TSGameState->GetTeamIdForCharacter(PlayerWhoHitBall);

		int ReceivingTeamId = (LastPlayerToHitTeamId) ? 0 : 1;

		FTeamData ReceivingTeamData = TSGameState->GetTeamById(ReceivingTeamId);

		//NOTE(achester): Purposely disabling distance indicator stuff after implementing swing forgiveness
		//TODO(achester): Refactor for doubles
		/*if (ReceivingTeamData.AssignedCharacters.Num() > 0 && ReceivingTeamData.AssignedCharacters[0]->IsLocallyControlled())
		{
			DistanceIndicatorComp->StartVisualizingDistance(ReceivingTeamData.AssignedCharacters[0]);
		}
		else
		{
			DistanceIndicatorComp->StopVisualizingDistance();
		}
		
		if (DynamicBallMat)
		{
			DynamicBallMat->SetScalarParameterValue(TEXT("Emission"), 0.f);
		}*/
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

		if (DynamicBallMat)
		{
			DynamicBallMat->SetScalarParameterValue(TEXT("Emission"), 0.f);
		}

		if (DistanceIndicatorComp->IsVisualizingDistance())
		{
			DistanceIndicatorComp->StopVisualizingDistance();
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

void ATennisBall::SetBallTrailColor(FGameplayTag argShotTypeTag)
{
	if (!BallTrailParticleEffect)
	{
		return;
	}

	UDataTable* ShotTypeColorMappingDT = UTrajectoryDataProvider::GetDefaultColorMappingTable();

	if (ShotTypeColorMappingDT)
	{
		for (auto Row : ShotTypeColorMappingDT->GetRowMap())
		{
			FShotTypeColorMapping* ColorMapping = reinterpret_cast<FShotTypeColorMapping*>(Row.Value);

			ensureMsgf(ColorMapping, TEXT("Wrong row struct in DT"));

			if (ColorMapping->ShotTypeTag == argShotTypeTag)
			{
				BallTrailParticleEffect->SetColorParameter(TEXT("ShotTypeColor"), ColorMapping->ShotColor);
				return;
			}
		}
	}
}

void ATennisBall::HandleDistanceIndicatorTargetReached()
{
	if (DynamicBallMat)
	{
		DynamicBallMat->SetScalarParameterValue(TEXT("Emission"), 30.f);
	}
}

float ATennisBall::DefaultBallRadius = 3.25f;
