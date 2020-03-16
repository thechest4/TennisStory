// Fill out your copyright notice in the Description page of Project Settings.

#include "GameMenuWidget.h"
#include "Player/TennisStoryPlayerController.h"

void UGameMenuWidget::HideGameMenu()
{
	APlayerController* OwningPlayer = GetOwningPlayer();
	ATennisStoryPlayerController* TSPC = Cast<ATennisStoryPlayerController>(OwningPlayer);

	if (TSPC)
	{
		TSPC->HideGameMenu();
	}
	else
	{
		RemoveFromViewport();
	}
}
