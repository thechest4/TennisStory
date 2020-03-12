// Fill out your copyright notice in the Description page of Project Settings.

#include "BounceLocationMarker.h"
#include "Particles/ParticleSystemComponent.h"

ABounceLocationMarker::ABounceLocationMarker()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	ParticleSystemComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particle System"));
	ParticleSystemComp->SetupAttachment(RootComponent);
}

void ABounceLocationMarker::BeginPlay()
{
	Super::BeginPlay();
	
	RootComponent->SetVisibility(false, true);
}

void ABounceLocationMarker::Multicast_ShowMarkerAtLocation_Implementation(FVector Location, float Duration)
{
	SetActorLocation(Location);
	RootComponent->SetVisibility(true, true);

	GetWorldTimerManager().ClearAllTimersForObject(this);
	
	ParticleSystemComp->ActivateSystem();

	static const float MAX_DURATION = 5.f;

	Duration = (Duration > 0.f) ? Duration : MAX_DURATION;

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ABounceLocationMarker::HideMarker, Duration);
}

void ABounceLocationMarker::HideMarker()
{
	ParticleSystemComp->DeactivateSystem();
	RootComponent->SetVisibility(false, true);
}
