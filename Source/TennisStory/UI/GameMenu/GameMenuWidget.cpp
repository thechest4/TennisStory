// Fill out your copyright notice in the Description page of Project Settings.

#include "GameMenuWidget.h"
#include "Player/TennisStoryPlayerController.h"

void UGameMenuWidget::BroadcastWantsCloseEvent()
{
	GameMenuWantsCloseEvent.ExecuteIfBound();
}
