// Fill out your copyright notice in the Description page of Project Settings.

#include "SmashZone.h"
#include <Components/SphereComponent.h>
#include "Player/TennisStoryCharacter.h"

ASmashZone::ASmashZone()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	PlayerDetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PlayerDetectionSphere"));
	PlayerDetectionSphere->SetupAttachment(RootComponent);
	PlayerDetectionSphere->SetCollisionProfileName(TEXT("SmashZone"));
	PlayerDetectionSphere->SetGenerateOverlapEvents(true);
	PlayerDetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ASmashZone::HandleSphereBeginOverlap);
	PlayerDetectionSphere->OnComponentEndOverlap.AddDynamic(this, &ASmashZone::HandleSphereEndOverlap);

	RingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RingMeshComp"));
	RingMesh->SetupAttachment(RootComponent);
	RingMesh->SetCollisionProfileName(TEXT("NoCollision"));
}

// Called when the game starts or when spawned
void ASmashZone::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASmashZone::HandleSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ATennisStoryCharacter* TSChar = Cast<ATennisStoryCharacter>(OtherActor);
	if (TSChar)
	{
		RingMesh->SetMaterial(0, OccupiedMaterial);

		TSChar->EnableSmashAbility();

		TWeakObjectPtr<ATennisStoryCharacter> WeakTSChar = TSChar;
		AffectedPlayers.Add(WeakTSChar);
	}
}

void ASmashZone::HandleSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ATennisStoryCharacter* TSChar = Cast<ATennisStoryCharacter>(OtherActor);
	if (TSChar)
	{
		TWeakObjectPtr<ATennisStoryCharacter> WeakTSChar = TSChar;
		AffectedPlayers.Remove(WeakTSChar);

		if (!AffectedPlayers.Num())
		{
			RingMesh->SetMaterial(0, NormalMaterial);

			TSChar->DisableSmashAbility();
		}
	}
}

