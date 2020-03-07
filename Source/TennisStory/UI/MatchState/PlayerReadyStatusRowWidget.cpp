// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerReadyStatusRowWidget.h"
#include "Player/TennisStoryPlayerState.h"

void UPlayerReadyStatusRowWidget::SetUpRow(ATennisStoryPlayerState* PlayerState)
{
	AssociatedPlayerState = PlayerState;
	AssociatedPlayerState->OnReadyStateUpdated().AddUObject(this, &UPlayerReadyStatusRowWidget::HandlePlayerReadyStateUpdated);
	HandlePlayerReadyStateUpdated(AssociatedPlayerState);
}

void UPlayerReadyStatusRowWidget::CleanUpRow()
{
	AssociatedPlayerState->OnReadyStateUpdated().RemoveAll(this);
	AssociatedPlayerState = nullptr;
}

bool UPlayerReadyStatusRowWidget::IsAssociatedWithPlayerState(ATennisStoryPlayerState* PlayerState)
{
	return PlayerState && AssociatedPlayerState == PlayerState;
}
