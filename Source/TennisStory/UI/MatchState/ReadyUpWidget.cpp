// Fill out your copyright notice in the Description page of Project Settings.

#include "ReadyUpWidget.h"
#include "Player/TennisStoryPlayerState.h"
#include "Player/TennisStoryPlayerController.h"

void UReadyUpWidget::SetUpWidget()
{
	TryGetLocalPlayerState();
}

void UReadyUpWidget::CleanUpWidget()
{
	if (LocalPlayerState)
	{
		LocalPlayerState->OnReadyStateUpdated().RemoveAll(this);
		LocalPlayerState = nullptr;
	}
}

void UReadyUpWidget::TryGetLocalPlayerState()
{
	if (LocalPlayerState)
	{
		return;
	}

	for (FConstPlayerControllerIterator ControllerItr = GetWorld()->GetPlayerControllerIterator(); ControllerItr; ControllerItr++)
	{
		ATennisStoryPlayerController* TSPC = Cast<ATennisStoryPlayerController>(ControllerItr->Get());
		if (TSPC && TSPC->IsLocalPlayerController())
		{
			LocalPlayerState = TSPC->GetPlayerState<ATennisStoryPlayerState>();
			if (LocalPlayerState)
			{
				LocalPlayerState->OnReadyStateUpdated().AddUObject(this, &UReadyUpWidget::HandleReadyStateUpdated);
				HandleReadyStateUpdated(LocalPlayerState);
			}
		}
	}
}

void UReadyUpWidget::ToggleReadyState()
{
	if (LocalPlayerState)
	{
		LocalPlayerState->Server_UpdateIsReady(!LocalPlayerState->IsReady());
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("UReadyUpWidget::ToggleReadyState - LocalPlayerState was null"));
	}
}
