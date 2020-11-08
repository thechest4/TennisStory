// Fill out your copyright notice in the Description page of Project Settings.

#include "TennisNetActor.h"
#include <Components/BoxComponent.h>

ATennisNetActor::ATennisNetActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = NetMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Net Static Mesh")); 
	NetMesh->SetCollisionProfileName(TEXT("NoCollision"));

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collision Comp"));
	BoxCollision->SetupAttachment(RootComponent);
	BoxCollision->SetCollisionProfileName(TEXT("TennisNet"));
}

