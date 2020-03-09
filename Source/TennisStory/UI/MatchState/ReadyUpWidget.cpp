// Fill out your copyright notice in the Description page of Project Settings.

#include "ReadyUpWidget.h"
#include "Player/TennisStoryPlayerState.h"
#include "Player/TennisStoryPlayerController.h"

void UReadyUpWidget::SetUpWidget()
{
	//NOTE(achester): the call below can actually fail to get a playerstate if on a client where the playerstates have not been fully replicated yet.
	//As a result, we simply call the function again the first time the widget is interacted with since by then there should be no issues.  However this should probably change
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
