// Fill out your copyright notice in the Description page of Project Settings.


#include "TennisBall.h"
#include "TennisStoryGameMode.h"

ATennisBall::ATennisBall()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATennisBall::BeginPlay()
{
	ATennisStoryGameMode* GameMode = GetWorld()->GetAuthGameMode<ATennisStoryGameMode>();
	if (GameMode)
	{
		GameMode->SetCurrentTennisBall(this);
	}
}

