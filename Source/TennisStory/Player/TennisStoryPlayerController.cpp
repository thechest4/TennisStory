// Fill out your copyright notice in the Description page of Project Settings.

#include "TennisStoryPlayerController.h"
#include "Net/UnrealNetwork.h"

void ATennisStoryPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetControlRotation(FRotator::ZeroRotator);
}
