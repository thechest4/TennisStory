// Fill out your copyright notice in the Description page of Project Settings.


#include "TennisRacquet.h"
#include "Components/CapsuleComponent.h"

ATennisRacquet::ATennisRacquet()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = RacquetMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RacquetMeshComp"));

	OverlapDetectionComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("OverlapDetectionComp"));
	OverlapDetectionComp->SetupAttachment(RootComponent, RacquetHeadSocket);
}

