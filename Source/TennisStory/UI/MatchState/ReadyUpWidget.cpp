// Fill out your copyright notice in the Description page of Project Settings.

#include "ReadyUpWidget.h"
#include "Player/TennisStoryPlayerState.h"
#include "Player/TennisStoryPlayerController.h"

void UReadyUpWidget::SetUpWidget()
{
	for (FConstPlayerControllerIterator ControllerItr = GetWorld()->GetPlayerControllerIterator(); ControllerItr; ControllerItr++)
	{
		ATennisStoryPlayerController* TSPC = Cast<ATennisStoryPlayerController>(ControllerItr->Get());
		if (TSPC && TSPC->IsLocalPlayerController())
		{
			LocalPlayerState = TSPC->GetPlayerState<ATennisStoryPlayerState>();
			LocalPlayerState->OnReadyStateUpdated().AddUObject(this, &UReadyUpWidget::HandleReadyStateUpdated);
		}
	}

	if (!LocalPlayerState)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("UReadyUpWidget::SetUpWidget - Did not find a LocalPlayerState"));
	}

	EnableWidgetInteraction();
	HandleReadyStateUpdated(LocalPlayerState);
}

void UReadyUpWidget::CleanUpWidget()
{
	if (LocalPlayerState)
	{
		LocalPlayerState->OnReadyStateUpdated().RemoveAll(this);
		LocalPlayerState = nullptr;
	}
}

void UReadyUpWidget::ToggleReadyState()
{
	if (LocalPlayerState)
	{
		LocalPlayerState->Client_UpdateIsReady(!LocalPlayerState->IsReady());
		DisableWidgetInteraction();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("UReadyUpWidget::ToggleReadyState - LocalPlayerState was null"));
	}
}

void UReadyUpWidget::HandleReadyStateUpdated_Implementation(ATennisStoryPlayerState* PlayerState)
{
	EnableWidgetInteraction();
}
