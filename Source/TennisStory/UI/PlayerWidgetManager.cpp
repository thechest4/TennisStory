// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerWidgetManager.h"
#include "UI/GameMenu/GameMenuWidget.h"
#include "UI/MatchState/PlayerReadyStatusWidget.h"
#include "UI/MatchState/ReadyUpWidget.h"

void UPlayerWidgetManager::ShowGameMenu()
{
	ensureMsgf(GameMenuWidgetObject, TEXT("UPlayerWidgetManager::ShowGameMenu - GameMenuWidgetObject was null!"));

	if (!GameMenuWidgetObject)
	{
		return;
	}

	GameMenuWidgetObject->SetVisibility(ESlateVisibility::Visible);

	SetPlayerInputToUIMode();
}

void UPlayerWidgetManager::HideGameMenu()
{
	ensureMsgf(GameMenuWidgetObject, TEXT("UPlayerWidgetManager::HideGameMenu - GameMenuWidgetObject was null!"));
	
	if (!GameMenuWidgetObject)
	{
		return;
	}

	GameMenuWidgetObject->SetVisibility(ESlateVisibility::Collapsed);

	SetPlayerInputToGameMode();
}

void UPlayerWidgetManager::ShowReadyWidgets()
{
	ensureMsgf(ReadyStateWidgetObject, TEXT("UPlayerWidgetManager::ShowReadyWidgets - ReadyStateWidgetObject was null!"));
	ensureMsgf(ReadyUpWidgetObject, TEXT("UPlayerWidgetManager::ShowReadyWidgets - ReadyUpWidgetObject was null!"));

	if (!ReadyStateWidgetObject || !ReadyUpWidgetObject)
	{
		return;
	}

	ReadyStateWidgetObject->SetVisibility(ESlateVisibility::Visible);
	ReadyUpWidgetObject->SetVisibility(ESlateVisibility::Visible);
}

void UPlayerWidgetManager::HideReadyWidgets()
{
	ensureMsgf(ReadyStateWidgetObject, TEXT("UPlayerWidgetManager::HideReadyWidgets - ReadyStateWidgetObject was null!"));
	ensureMsgf(ReadyUpWidgetObject, TEXT("UPlayerWidgetManager::HideReadyWidgets - ReadyUpWidgetObject was null!"));
	
	if (!ReadyStateWidgetObject || !ReadyUpWidgetObject)
	{
		return;
	}
	
	ReadyStateWidgetObject->SetVisibility(ESlateVisibility::Collapsed);
	ReadyUpWidgetObject->SetVisibility(ESlateVisibility::Collapsed);
}

void UPlayerWidgetManager::HideAllWidgets()
{
	for (int i = 0; i < AllWidgets.Num(); i++)
	{
		AllWidgets[i]->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPlayerWidgetManager::NativeOnInitialized()
{
	CatalogWidgetObjects();

	ensureMsgf(GameMenuWidgetObject && ReadyStateWidgetObject && ReadyUpWidgetObject, TEXT("UPlayerWidgetManager::NativeOnInitialized - Not all widgets were catalogued properly"));

	AllWidgets.Add(Cast<UUserWidget>(GameMenuWidgetObject));
	AllWidgets.Add(Cast<UUserWidget>(ReadyStateWidgetObject));
	AllWidgets.Add(Cast<UUserWidget>(ReadyUpWidgetObject));

	HideAllWidgets();

	Super::NativeOnInitialized();
}

void UPlayerWidgetManager::SetPlayerInputToUIMode()
{
	APlayerController* OwningPlayer = GetOwningPlayer();

	checkf(OwningPlayer, TEXT("UPlayerWidgetManager::SetPlayerInputToUIMode - OwningPlayer was null"))

	FInputModeUIOnly UIInputOnly;
	//UIInputOnly.SetWidgetToFocus(GameMenuWidgetObject->TakeWidget());

	OwningPlayer->SetInputMode(UIInputOnly);
	OwningPlayer->bShowMouseCursor = true;
}

void UPlayerWidgetManager::SetPlayerInputToGameMode()
{
	APlayerController* OwningPlayer = GetOwningPlayer();

	checkf(GetOwningPlayer(), TEXT("UPlayerWidgetManager::SetPlayerInputToGameMode - OwningPlayer was null"))

	FInputModeGameOnly GameInputOnly;
	OwningPlayer->SetInputMode(GameInputOnly);
	OwningPlayer->bShowMouseCursor = false;
}
