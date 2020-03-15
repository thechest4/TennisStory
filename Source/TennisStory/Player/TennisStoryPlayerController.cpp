// Fill out your copyright notice in the Description page of Project Settings.

#include "TennisStoryPlayerController.h"
#include "Net/UnrealNetwork.h"

void ATennisStoryPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	//NOTE(achester): This is important so that the far-court player still controls like the near court player despite facing toward the camera.
	//This will probably need to change to support different cameras
	SetControlRotation(FRotator::ZeroRotator);
}
