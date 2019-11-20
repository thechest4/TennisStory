// Fill out your copyright notice in the Description page of Project Settings.


#include "TennisRacquet.h"

ATennisRacquet::ATennisRacquet()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = RacquetMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RacquetMeshComp"));
	RacquetMesh->SetCollisionProfileName(TEXT("NoCollision"));
}

