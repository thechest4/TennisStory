// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerReadyStatusWidget.h"
#include "TennisStoryGameState.h"
#include "Player/TennisStoryPlayerState.h"

void UPlayerReadyStatusWidget::SetUpWidget()
{
	ATennisStoryGameState* TSGS = GetWorld()->GetGameState<ATennisStoryGameState>();

	checkf(TSGS, TEXT("UPlayerReadyStatusWidget::SetUpWidget - Couldn't get TSGS"))

	for (int i = 0; i < TSGS->PlayerArray.Num(); i++)
	{
		ATennisStoryPlayerState* TSPS = Cast<ATennisStoryPlayerState>(TSGS->PlayerArray[i]);

		checkf(TSGS, TEXT("UPlayerReadyStatusWidget::SetUpWidget - TSPS was null"))

		AddPlayerStateWidget(TSPS);
	}

	TSGS->OnPlayerStateAdded().AddUObject(this, &UPlayerReadyStatusWidget::AddPlayerStateWidget);
	TSGS->OnPlayerStateRemoved().AddUObject(this, &UPlayerReadyStatusWidget::RemovePlayerStateWidget);
}

void UPlayerReadyStatusWidget::CleanUpWidget()
{
	ATennisStoryGameState* TSGS = GetWorld()->GetGameState<ATennisStoryGameState>();

	checkf(TSGS, TEXT("UPlayerReadyStatusWidget::CleanUpWidget - Couldn't get TSGS"))

	for (int i = 0; i < TSGS->PlayerArray.Num(); i++)
	{
		ATennisStoryPlayerState* TSPS = Cast<ATennisStoryPlayerState>(TSGS->PlayerArray[i]);

		checkf(TSGS, TEXT("UPlayerReadyStatusWidget::CleanUpWidget - TSPS was null"))

		RemovePlayerStateWidget(TSPS);
	}

	TSGS->OnPlayerStateAdded().RemoveAll(this);
	TSGS->OnPlayerStateRemoved().RemoveAll(this);
}
