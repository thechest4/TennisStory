// Fill out your copyright notice in the Description page of Project Settings.

#include "TennisStoryPlayerState.h"
#include "Net/UnrealNetwork.h"

ATennisStoryPlayerState::ATennisStoryPlayerState(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	bIsReady = false;
}

void ATennisStoryPlayerState::Client_UpdateIsReady_Implementation(bool bNewReady)
{
	bIsReady = bNewReady;
}

void ATennisStoryPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATennisStoryPlayerState, bIsReady);
}

void ATennisStoryPlayerState::OnRep_IsReady()
{

}
