// Fill out your copyright notice in the Description page of Project Settings.

#include "TennisStoryPlayerController.h"
#include "Net/UnrealNetwork.h"

void ATennisStoryPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATennisStoryPlayerController, PlayerNumber);
}
